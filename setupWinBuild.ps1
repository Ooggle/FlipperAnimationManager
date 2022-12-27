#### DO NOT REMOVE THIS FILE: THIS IS USED FOR CI BUILDING! #### 

# SDL Url
$url = "https://github.com/libsdl-org/SDL/releases/download/release-2.26.1/SDL2-2.26.1.zip"

# Set the name of the zip file and the target directory
$zipFile = "SDL.zip"
$targetDirectory = "tmp_sdl"

# Download the SDL ZIP file
Invoke-WebRequest -Uri $url -OutFile $zipFile

# Extract the SDL file
Expand-Archive -Path $zipFile -DestinationPath $targetDirectory

# Set the source and target directories for the copy operation 
$sourceDirectory = "$targetDirectory\include"
$targetDirectory = "."

# Copy the contents of \include
Get-ChildItem $sourceDirectory | Copy-Item -Destination $targetDirectory

# Remove the tmp_sdl folder
Remove-Item -Recurse -Force $targetDirectory
