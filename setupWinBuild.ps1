#### DO NOT REMOVE THIS FILE: THIS IS USED FOR CI BUILDING! #### 

# SDL Url
$url = "https://github.com/libsdl-org/SDL/releases/download/release-2.26.1/SDL2-2.26.1.zip"

# Set the name of the zip file and the target directory
$zipFile = "SDL.zip"
$targetDirectory = "tmp_sdl"

# Download the SDL ZIP file
Write-Output "Downloading SDL"
$response = Invoke-WebRequest -Uri $url -OutFile $zipFile
Write-Output "Done, server reply: $($response.StatusCode)"

# Extract the SDL file
Write-Output "Extracting..."
Expand-Archive -Path $zipFile -DestinationPath $targetDirectory

# Set the source and target directories for the copy operation 
Write-Output "Moving .h files..."
$sourceDirectory = "$targetDirectory\include"
$targetDirectory = "."

# Copy the contents of \include
Get-ChildItem $sourceDirectory | Copy-Item -Destination $targetDirectory


# Remove the tmp_sdl folder
Write-Output "Removing tmp_sdl"
Remove-Item -Recurse -Force $targetDirectory

Write-Output "DONE! SDL is ready for the ci runner. Continuing...."
