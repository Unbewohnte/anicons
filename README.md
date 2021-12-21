# Anicons
## extract icons from .ani RIFF files

### Build
`make` or `g++ src/anicons.cpp -o anicons`

### Usage
`anicons [PATH_TO_THE_FILE]`

will search through each chunk of the RIFF file and
save the contents of each encountered "icon" chunk