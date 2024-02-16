"""
Python Script for CUGL Projects

After years of supporting student projects, we got tired of supporting all the different
IDE projects. Particularly because they were so brittle and students refused to read
the directions on how to change them properly.

Originally, we had hoped CMake would solve the problem. However, CMake is an absolute
mess when it comes to mobile builds. While there are some experimental iOS generators,
they are not reliable and they completely fall apart with respect to code signing.
The situation is even worse on Android where there is no generator support for Android
Studio, and CMake cannot modify any of the Java configuration files (which is where most
of the application specific issues are). So while CMake is fine on desktop platforms
(and is the ideal build system for Linux), we had to abandon it.

This Python script is the compromise. For a given CUGL project, we generate the project
files for each target platform. It is designed so that users can safely edit code without
regenerating the project files. However major changes to the project (even something as
simple as making a new icon) should rebuild the project files.

To work properly, the SDL project must have a config.yml in its root directory. See the
documentation for the format of this YML file.

Author: Walker M. White
Date:   08/29/22
"""
import os, os.path
import argparse
import re
import scripts.iconify as iconify
import scripts.util as util

# The support platforms
PLATFORMS = { 'android' : 'Android build via Android Studio',
              'apple' : 'Both macOS and iOS build via XCode',
              'cmake' : 'Experimental Linux build via CMake',
              'macos' : 'macOS only build via XCode',
              'ios' : 'iOS only build via XCode',
              'windows' : 'Windows build via Visual Studio'
            }


# The allowed orientations
ORIENTATIONS = { 'portrait' : 'portrait mode only, home at the bottom (notch at top)',
                 'portrait-flipped' : 'portrait mode only, home at the top (notch at bottom)',
                 'portrait-either' : 'portrait mode only, but with direction depending on device orientiation',
                 'landscape' : 'landscape mode only, home at the right (notch at left)',
                 'landscape-flipped' : 'landscape mode only, home at the left (notch at right)',
                 'landscape-either' : 'landscape mode only, but with direction depending on device orientiation',
                 'multidirectional' : 'either portrait or landsscape mode, but not flipped',
                 'omindirectional' : 'all possible modes are allowed, depending on device orientiation'
                 }


def setup():
    """
    Returns an initialized parser for this script.
    
    :return: The parsed args
    :rtype:  ``Namespace``
    """
    parser = argparse.ArgumentParser(description='Build an SDL2 project.')
    parser.add_argument('project', type=str, help='The project directory')
    parser.add_argument('-b', '--build', type=str, help='The build directory')
    parser.add_argument('-c', '--config', type=str, help='The configuration file')
    parser.add_argument('-t', '--target', type=str,
                        choices=['android','apple','macos','ios','windows','cmake'],
                        help='The active build target')
    return parser.parse_args()


def get_config(args):
    """
    Returns the config settings for the given project
    
    Any CUGL project must have a config.yml in the root directory in order for the
    build system to work properly. This method parses that YML file and converts it
    into a Python data structure. If there is no config.yml, this method returns None.
    
    :param target: The path to the CUGL project
    :type data:  ``str``
    
    :return: a Python representation of the YML file
    :rtype:  ``dict``
    """
    import yaml
    
    if not args.config is None:
        config = args.config
    else:
        config = os.path.join(args.project,'config.yml')
    if not os.path.exists(config):
        raise FileNotFoundError('File %s does not exist.' % repr(config))
    
    data = None
    with open(config, encoding = 'utf-8') as file:
        data = file.read()
        data = yaml.load(data,Loader=yaml.Loader)
    
    if not data is None:
        data['uuids'] = util.UUIDService()
        if util.entry_has_path(data,'suffix') and data['suffix']:
            if type(data['suffix']) == bool:
                uuid = data['uuids'].getAppleUUID(os.path.abspath(config))
                uuid = data['uuids'].applyPrefix('G',uuid)[:8]
                data['suffix'] = uuid
                util.file_replace_after(config,{'suffix:':' '+uuid})
            data['appid'] += '.'+data['suffix']
        
        # Now add computed attributes
        data['root'] = os.path.abspath(args.project)
        data['cugl'] = os.path.split(__file__)[0]
        
        if not args.build is None:
            data['build'] = os.path.abspath(args.build)
        else:
            data['build'] = os.path.normpath(os.path.join(data['root'],data['build']))
        
        data['build_to_cugl'] = os.path.relpath(data['cugl'],data['build'])
        data['build_to_root'] = os.path.relpath(data['root'],data['build'])
    
    return data


def snake2Camel(text):
    """
    Returns a (capitalized) version of text in camel case
    
    All underscores are removed, and capital letters are placed at underscore boundaries.
    
    https://www.geeksforgeeks.org/python-convert-snake-case-string-to-camel-case/
    
    :param text: The text in snake case to convert
    :type text:  ``str``
    
    :return: a (capitalized) version of text in camel case
    :rtype:  ``str``
    """
    temp = text.split('_')
    return ''.join(ele[:1].upper()+ele[1:] for ele in temp)


def normalize(config,args):
    """
    Normalizes the config file to contain missing data
    
    :param config: The project configuration settings
    :type config:  ``dict``
    """
    if not 'name' in config:
        config['name'] = 'Game'
    if not 'version' in config:
        config['version'] = '1.0'
    
    pattern = re.compile('[^\w_]+')
    if not 'short' in config:
        config['short'] = pattern.sub('_',config['name'])
    elif not config['short'].isidentifier():
        config['short'] = pattern.sub('_',config['short'])
    
    camel = snake2Camel(config['short'])
    if camel == '':
        camel = 'Game'
    elif not camel[0].isalpha():
        camel = 'G'+camel
    config['camel'] = camel
    
    # Add other missing information
    if not 'orientation' in config:
        config['orientation'] = 'landscape'
    else:
        config['orientation'] = config['orientation'].lower()
    
    if args.target:
        config['targets'] = [args.target]
    elif 'targets' in config:
        if type(config['targets']) == list:
            config['targets'] = list(map(lambda x: x.lower(), config['targets']))
        elif type(config['targets']) == str:
            config['targets'] = [ config['targets'].lower() ]
        else:
            config['targets'] = []
    else:
        config['targets'] = []
    
    # We only allow one of these
    if 'apple' in config['targets']:
        if 'ios' in config['targets']:
            config['targets'].remove('ios')
        if 'macos' in config['targets']:
            config['targets'].remove('macos')
    elif 'macos' in config['targets'] and 'ios' in config['targets']:
        config['targets'].remove('ios')
        config['targets'].remove('macos')
        config['targets'].append('apple')
    
    if not 'assets' in config:
        config['assets'] = 'assets'


def validate(config):
    """
    Returns False (with an error message) if the config file properties are wrong
    
    :param config: The project configuration settings
    :type config:  ``dict``
    
    :return: False (with an error message) if the config file properties are wrong
    :rtype:  ``bool``
    """
    if not 'appid' in config:
        print('ERROR: Configuration file is missing the appid')
        return False
    elif not '.' in config['appid']:
        print('ERROR: appid value must have at least one period: %s' % repr(config['appid']))
        return False
    
    if not 'sources' in config or not config['sources']:
        print('ERROR: Configuration file has no source files')
        return False
    
    if 'orientation' in config:
        if not config['orientation'] in ORIENTATIONS:
            print('ERROR: %s is not a valid orientiation' % repr(config['orientation']))
            print('Choose one of the following: ')
            pad = max(map(lambda x : len(x), ORIENTATIONS))+2
            for key in ORIENTATIONS:
                prompt = (key+': ')
                prompt += ' '*(pad-len(prompt))
                print('- %s %s' % (prompt,ORIENTATIONS[key]))
            return False
    
    if 'targets' in config and config['targets']:
        for target in config['targets']:
            if not target in PLATFORMS:
                print('ERROR: %s is not a valid platform' % repr(target))
                print('Choose one of the following: ')
                pad = max(map(lambda x : len(x), PLATFORMS))+2
                for key in PLATFORMS:
                    prompt = (key+': ')
                    prompt += ' '*(pad-len(prompt))
                    print('- %s %s' % (prompt,PLATFORMS[key]))
                return False
    
    return True


def expand_sources(config):
    """
    Updates the config to include all of the project sources
    
    The sources are placed into the entry 'source_tree' as a filetree. That is
    a recursive dictionary where each each key is a path relative to parent (top
    level keys are relative to the project root). The leaves are two-element
    tuples were the first element is the file and the second is a target. The
    target 'all' is for all targets.
    
    :param config: The project configuration settings
    :type config:  ``dict``
    """
    root = config['root']
    
    filetree = {}
    # Get the main sources
    for item in config['sources']:
        files = util.expand_wildcards(root,item)
        util.insert_filetree(filetree,files,'all')
    
    # Get the auxiliary sources
    for target in ['android','apple','macos','ios','windows','cmake']:
        if target in config and 'sources' in config[target]:
            if type(config[target]['sources']) == list:
                for item in config[target]['sources']:
                    files = util.expand_wildcards(root,item)
                    util.insert_filetree(filetree,files,target)
            elif config[target]['sources']:
                files = util.expand_wildcards(root,config[target]['sources'])
                util.insert_filetree(filetree,files,target)
    
    config['source_tree'] = filetree


def expand_assets(config):
    """
    Updates the config to include all of the assets
    
    The assets are placed into the entry 'asset_list' as a list. The elements are
    two-element tuples were the first element is the file and the second is an
    indication of 'file' or 'directory'.
    
    :param config: The project configuration settings
    :type config:  ``dict``
    """
    assetlst = []
    assetdir = config['assets'].split('/')
    if len(assetdir) > 0 and assetdir[0] == '':
        assetdir = assetdir[1:]
    
    root = config['root']
    path = os.path.join(root,*assetdir)
    if os.path.isdir(path):
        for item in os.listdir(path):
            if item[0] != '.':
                if os.path.isdir(os.path.join(path,item)):
                    assetlst.append((item,'directory'))
                else:
                    assetlst.append((item,'file'))
    
    config['asset_list'] = assetlst


def expand_includes(config):
    """
    Updates the config to include all of the include directories

    Include directories are specified as a string of space separated directories.
    The directories are placed into the entry 'include_dict' as a dict. The keys are
    the targets.

    :param config: The project configuration settings
    :type config:  ``dict``
    """
    root = util.posix_to_path(config['root'])
    
    result = {}
    includes = []
    if 'includes' in config and config['includes']:
        if type(config['includes']) == list:
            for item in config['includes']:
                files = util.expand_wildcards(root,item,'directory')
                includes.extend(map(util.posix_to_path,files))
        elif config['includes']:
            files = util.expand_wildcards(root,config['includes'],'directory')
            includes.extend(map(util.posix_to_path,files))
        result['all'] = includes
    
    # Get the auxiliary sources
    for target in ['android','apple','macos','ios','windows','cmake']:
        includes = []
        if target in config and 'includes' in config[target]:
            if type(config[target]['includes']) == list:
                for item in config[target]['includes']:
                    files = util.expand_wildcards(root,item,'directory')
                    includes.extend(map(util.posix_to_path,files))
            elif config[target]['includes']:
                files = util.expand_wildcards(root,config[target]['includes'],'directory')
                includes.extend(map(util.posix_to_path,files))
        result[target] = includes
    
    config['include_dict'] = result


def build_icon(config):
    """
    Creates an icon maker if icon information is present
    
    :param config: The project configuration settings
    :type config:  ``dict``
    """
    if not 'icon' in config:
        return
    
    if type(config['icon']) == str:
        foreground = config['icon']
        background = 'ffffff'
        transparent = False
        rounded = True
    elif type(config['icon']) == dict:
        foreground = None if not 'image' in config['icon'] else config['icon']['image']
        background = '#fffff' if not 'background' in config['icon'] else config['icon']['background']
        transparent = 'transparent' in config['icon'] and config['icon']['transparent']
        rounded = not 'rounded' in config['icon'] or config['icon']['rounded']
    
    if foreground:
        expand = util.posix_split(foreground)
        if not os.path.ismount(expand[0]):
            foreground = os.path.join(config['root'],*expand)
        config['icon'] = iconify.IconMaker(foreground,background,transparent,rounded)


def main():
    """
    Runs the build script
    """
    args = setup()
    config = get_config(args)
    normalize(config,args)
    if not validate(config) or not config['targets']:
        return
    
    print('Locating assets and source files')
    expand_sources(config)
    expand_assets(config)
    expand_includes(config)
    build_icon(config)
    
    for target in config['targets']:
        if target == 'cmake':
            import scripts.cmake
            scripts.cmake.make(config)
        if target == 'android':
            import scripts.android
            scripts.android.make(config)
        if target in ['apple','macos','ios']:
            import scripts.apple
            scripts.apple.make(config)
        if target == 'windows':
            import scripts.windows
            scripts.windows.make(config)


if __name__ == '__main__':
    main()
