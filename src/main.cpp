#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#include "Conditions.h"

void InitLogging()
{
	auto path = logs::log_directory();
	if (!path)
		return;

	const auto plugin = SKSE::PluginDeclaration::GetSingleton();
	*path /= fmt::format("{}.log", plugin->GetName());

	std::vector<spdlog::sink_ptr> sinks{
		std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true),
		std::make_shared<spdlog::sinks::msvc_sink_mt>()
	};

#ifndef NDEBUG
	const auto level = spdlog::level::trace;
#else
	const auto level = spdlog::level::info;
#endif

	auto logger = std::make_shared<spdlog::logger>("global", sinks.begin(), sinks.end());
	logger->set_level(level);
	logger->flush_on(level);

	spdlog::set_default_logger(std::move(logger));
	spdlog::set_pattern("[%^%L%$] %v");
}

template <typename T>
void RegisterCondition()
{
	auto result = OAR_API::Conditions::AddCustomCondition<T>();
	switch (result)
	{
	case OAR_API::Conditions::APIResult::OK:
		logs::info("Registered {} condition!", T::CONDITION_NAME);
		break;
	case OAR_API::Conditions::APIResult::AlreadyRegistered:
		logs::warn("Condition {} is already registered!",
		           T::CONDITION_NAME);
		break;
	case OAR_API::Conditions::APIResult::Invalid:
		logs::error("Condition {} is invalid!", T::CONDITION_NAME);
		break;
	case OAR_API::Conditions::APIResult::Failed:
		logs::error("Failed to register condition {}!", T::CONDITION_NAME);
		break;
	}
}

void InitMessaging()
{
	logs::trace("Initializing messaging listener...");
	const auto intfc = SKSE::GetMessagingInterface();
	if (!intfc->RegisterListener([](SKSE::MessagingInterface::Message* a_msg)
	{
		if (a_msg->type == SKSE::MessagingInterface::kPostLoad)
		{
			OAR_API::Conditions::GetAPI(OAR_API::Conditions::InterfaceVersion::V2);
			if (g_oarConditionsInterface)
			{
				RegisterCondition<Conditions::DetectedByCondition>();
				RegisterCondition<Conditions::DetectsCondition>();
				RegisterCondition<Conditions::DetectionDistanceCondition>();
				RegisterCondition<Conditions::DetectionRelationshipCondition>();
				RegisterCondition<Conditions::DetectionAngleCondition>();
			}
			else
			{
				logs::error("Failed to request Open Animation Replacer API"sv);
			}
		}
	}))
	{
		stl::report_and_fail("Failed to initialize message listener.");
	}
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	InitLogging();

	const auto plugin = SKSE::PluginDeclaration::GetSingleton();
	logs::info("{} v{} is loading...", plugin->GetName(), plugin->GetVersion());

	SKSE::Init(a_skse);
	InitMessaging();

	logs::info("{} loaded.", plugin->GetName());

	return true;
}
