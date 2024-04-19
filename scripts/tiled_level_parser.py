'''
Script for obtaining an id to texture file mapping for a Tiled level file (.tmx).
Call this script on a .tmx file from the command line.
Make sure the tileset files referenced by the level are in the same place they were when you made the level so that the script can find them.
Overwrite the window_strings variable in GameplayController.cpp with the output to implement the mapping.
Note that the mapping is global to all levels, so make sure you make all levels from the same Tiled session with all tilesets imported.
	I may try to change this in the future.

Created: 4/15/24
Updated: 4/15/24
Author: Philip Martin
'''


import sys
import re # regular expressions :D

# verify args
if len(sys.argv) < 2:
	print("missing argument: [level file name]")
	print("usage: python tiled_level_parser.py [level file name]")
	print("[level file name] should be a .tmx in your Levels folder")
	exit()


level_file_name = sys.argv[1]
mapping = [] # map to texture file name based on index in this list


print("-------------------------------------------------")
print("Parsing file: " + level_file_name)
# read level file contents
level_file = open(level_file_name)
level_file_text = level_file.read()
print("-------------------------------------------------")



# find all tilesets referenced by this level
tileset_matches = re.findall('firstgid=".*" source=".*"', level_file_text)

print("Found tilesets:\n")
for tileset_match in tileset_matches:
	print(tileset_match)
print("-------------------------------------------------")



# open each tileset file and extract id data
print("Found textures by id:\n")
for tileset_match in tileset_matches:
	firstgid = int(re.match('firstgid="[^"]*"', tileset_match).group()[10:-1])
	tileset_file_name = re.search('source="[^"]*"', tileset_match).group()[8:-1]
	tileset_file = open(tileset_file_name)
	tileset_file_text = tileset_file.read()
	image_file_matches = re.findall('image width=".*" height=".*" source=".*"', tileset_file_text)
	i = 0
	for image_file_match in image_file_matches:
		image_file_match = re.search('source=".*"', image_file_match).group()[8:-1]
		image_file_name = re.search('/[^/]*\Z', image_file_match).group()[1:]
		image_file_no_extension = re.search('\A[^.]*', image_file_name).group()
		mapping.append(image_file_no_extension)
		print(str(firstgid + i) + ": " + image_file_name)
		i += 1
print("-------------------------------------------------")



# format and print the final mapping of textures
print("Final mapping:")
print("(update the window_strings variable in GameplayController.cpp with this)")
print()
mapping_string = 'string window_strings[' + str(len(mapping)) + '] = {' + str(mapping)[1:-1] + '};'
mapping_string = mapping_string.replace("'", '"')
print(mapping_string)
	