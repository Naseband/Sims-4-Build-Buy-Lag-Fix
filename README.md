# Sims-4-Build-Buy-Lag-Fix-ASI-
ASI Plugin version of the build/buy lag fix for The Sims 4 (super simple hack).

# What's the issue?
When entering or exiting the build/buy mode in the game, some players get really annoying FPS drops. These fix after some time, or by pressing ESC twice. Usually not a big deal, but if you switch around very frequently it's just annoying. This fixes it.

# Installation
You will need an ASI loader to load the plugin. I suggest Ultimate ASI Loader by ThirteenAG.

https://github.com/ThirteenAG/Ultimate-ASI-Loader/

Go to releases and download the x64 version of the ASI loader.
Next, download the release of this plugin.

- Extract dinput8.dll (the ASI loader) into *Sims 4 root directory\Game\Bin\* and rename it to *version.dll*.
- Extract the contents of *BBLagFix.zip* into the same directory. You can also build the plugin yourself, but you'll need the ini files (both version.ini and BBLagFix.ini).

Inside the scripts directory you will find *BBLagFix.ini*, which is used to configure the plugin.
If the game is updated you can simply copy the Address, Expected and Target values in the Patch section to make the plugin compatible with the new game version, or just redownload it. It will most likely not be neccessary to update the ASI itself.

# Patch for 1.87.40.1030
> Address = 0x140C8083D
> 
> Expected = 0xE081C71774
> 
> Target = 0xE081C717EB
