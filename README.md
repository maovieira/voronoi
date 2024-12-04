# Voronoi

## Overview

Voronoi is a console program developed as a GEOS library integration exercise to study library functionality. It processes GeoJSON files to generate Voronoi diagrams.

## Version

0.01

## Usage

```
Voronoi [-hv -f file (stdin)] [file]
```

### Options

- `-f file`: Specify filename to read (can be used with or without `-f`)
- `-h`: Display help information
- `-v`: Display version information

### Input

- Accepts GeoJSON files containing points
- Accepts multi geometry collections containint points (all other geometry types other than points will be ignored)
- Supports reading from stdin or specified file

### Output

- Writes Voronoi Diagram to stdout

## Building

cc src/voronoi.c -o ./bin/voronoi -l geos_c

> There is a dummy Makefile in the directory not used at all.

## Dependencies

- GEOS Library

## License

This project is licensed under the GPLv2.1 License - see the [LICENSE](LICENSE) file for details

## Purpose

Created solely as an educational exercise for studying GEOS library integration.
