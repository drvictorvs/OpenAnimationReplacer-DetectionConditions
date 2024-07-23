import os
import json

# Constants
your_directory_path = ''
keys_to_remove = [
    "Use the humanoid condition", "Is humanoid", "Use the gender condition",
    "Is Female", "Use the relationship condition", "Comparison",
    "Relationship [-4 = Archnemesis, .., 4 = Lover]",
    "Use the faction condition", "Is NOT", "Faction",
    "Use the actorbase condition", "Is NOT_2", "Actor base",
    "Use the distance condition", "Comparison_2", "Distance (centimeters)",
    "Use the keyword condition", "Is NOT_3", "Keyword",
    "Use the compare value condition", "Value A", "Comparison_3", "Value B",
    "negated"
]


def create_condition(condition_type, old_conditions, keys):
  return {
      "condition": condition_type,
      "requiredPlugin": "OpenAnimationReplacer-DetectionPlugin",
      "requiredVersion": old_conditions.get("requiredVersion", ""),
      **{
          key: old_conditions.get(key, False)
          for key in keys
      }
  }


def translate_conditions(old_conditions):
  condition_mappings = {
      "Use the humanoid condition": ("DetectionHumanoid", ["Is humanoid"]),
      "Use the relationship condition":
      ("DetectionRelationship",
       ["Comparison", "Relationship [-4 = Archnemesis, .., 4 = Lover]"]),
      "Use the faction condition": ("DetectionFaction", ["Is NOT", "Faction"]),
      "Use the distance condition":
      ("DetectionDistance", ["Comparison_2", "Distance (centimeters)"])
  }
  new_conditions = []
  for key, (condition_type, keys) in condition_mappings.items():
    if old_conditions.get(key, False):
      new_conditions.append(
          create_condition(condition_type, old_conditions, keys))
  return new_conditions


def process_condition(condition):
  if condition.get("condition").lower() in [
      "detects", "isdetectedby", "detected_by"
  ]:
    new_condition = {
        "condition": "DETECTED_BY" if condition.get("condition") == "IsDetectedBy" else condition.get("condition").upper(),
        "requiredPlugin": condition.get("requiredPlugin", ""),
        "requiredVersion": condition.get("requiredVersion", ""),
        "negated": condition.get("negated", False),
        "Conditions": translate_conditions(condition)
    }
    for key in keys_to_remove:
      condition.pop(key, None)
    condition.update(new_condition)
  elif condition.get("condition").lower() in ["and", "or"]:
    for sub_condition in condition.get("Conditions", []):
      process_condition(sub_condition)


def translate_file(file_path):
  with open(file_path, 'r') as file:
    data = json.load(file)

  print('Opening ' + file_path + '...')

  for condition in data.get("conditions", []):
    process_condition(condition)

  print('Saving ' + file_path + '...')

  with open(file_path, 'w') as file:
    json.dump(data, file, indent=4)

  print('Saved ' + file_path + '.')


def translate_directory(directory):
  for root, _, files in os.walk(directory):
    for file in files:
      if file.endswith(".json"):
        translate_file(os.path.join(root, file))


# Replace '' with the path to your directory
translate_directory(your_directory_path)
