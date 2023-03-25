#!/bin/bash

if [[ $UID != 0 ]]; then
    echo "Please run this script with sudo."
    exit 1
fi

run_install()
{
    read -p "Do you want to install missing libraries? [Y/n]: " answer
    ## Set the default value if no answer was given
    answer=${answer:Y}
	
    ## Exit if anything other than Yy
    ! [[ $answer =~ [Yy] ]] && exit 2
	
    ## Update packages
    echo Updating Package Database...
    apt-get update
    
    ## Auto accept install on packages if Y
    echo Installing Packages...
    apt-get install ${depends[@]} -y
}

## List of required libraries
depends=("cmake" "g++" "llvm-14-dev" "libclang-14-dev" "clang-14")

## Check for required libraries
echo Checking for required libraries
dpkg -s "${depends[@]}" >/dev/null 2>&1 || run_install

## Begin build
cmake -B ./Build
cd Build || exit 3
cmake --build . --config Release
