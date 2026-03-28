#!/usr/bin/env bash

# This script will generate a cyclomatic complexity report for all of the AWE Manager source code
# The report will be generated in .html, .csv, and .xml formats

usage()
{
    # Display Help
    echo "This script will calculate the cyclomatic complexity of all C or C++ code in the provided source directory (recursive) using the lizard tool."
    echo "See the lizard code repo for more details on the tool: https://github.com/terryyin/lizard"
    echo "awecore-common will be used as the source directory by default. If needed, the -s option can be used to pass in another source directory."
    echo "The output will be an .xml, .html, and .csv report of the cyclomatic complexity. By default, the output will be placed a in a folder called out."
    echo 
    echo
    echo "Syntax: calculate_cyclomatic_complexity [-s|o|h]"
    echo "options:"
    echo "s      Specify where the source files being analyzed for coverage are located" 
    echo "         If no directory is specified, then awecore-common is used by default"
    echo "o      Define where the output files should be created"
    echo "         Default is awecore-common/Source/Scripts/CyclomaticComplexity/out"
    echo "n      Select output file name. Will by 'cyclo.<ext>' by default"
    echo "h      Print this Help"
    echo
    exit
}

# Get default paths to awecore-common
scriptFile=$(realpath "$0")
scriptDir=$(dirname "$scriptFile")
sourceDir=$(cd $scriptDir/../../..; pwd)
outDir=$scriptDir/out
outName=cyclo

# Get user arguments
while getopts "s:o:n:h" opt; do
    case "${opt}" in
        s)
            sourceDir=$OPTARG
            ;;
        o)
            outDir=$OPTARG
            ;;
        n)
            outName=$OPTARG
            ;;            
        h | *)
            usage
            ;;
    esac
done

# Check user has necessary programs installed
# Need lizard package, and also jinja2 dependency
if ! command -v lizard &> /dev/null
then
    echo "lizard was not found - please run 'pip install lizard jinja2' and try again"
    exit
fi

# Cleanup output directory
rm -rf $outDir
mkdir -p $outDir

# Generate the paths we should be excluding from report
excludePaths="-x"$sourceDir/examples/*" -x"$sourceDir/tests/*" -x"$sourceDir/components/*/tests/*""

#echo "Excluding these paths from calculation: ${excludePaths}"

echo "Generating cyclomatic complexity reports"
echo "-- Using sourceDir: $sourceDir"
echo "-- Using outDir: $outDir"
echo "-- Using outName: $outName"
lizard -l cpp $sourceDir $excludePaths -o $outDir/$outName.html

lizard -l cpp $sourceDir $excludePaths -o $outDir/$outName.xml

# Need to manually print the headers for each column in .csv report
echo Lines of Code, Cyclomatic Complexity Number,Token,Arg Count,Length,Location,File,Function,Signature,Start Line,End Line > $outDir/$outName.csv
lizard -l cpp $sourceDir $excludePaths --csv >> $outDir/$outName.csv

echo "Successfully generated $outName.html/.xml/.csv cyclomatic complexity reports in $outDir"
