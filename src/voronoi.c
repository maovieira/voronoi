#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <geos_c.h>

#define PACKAGE "Voronoi"
#define VERSION "0.01"
#define NOPTS 3


void notice(const char *fmt, ...) {
    printf("Notice: %s\n", fmt);
}

void error(const char *fmt, ...) {
    printf("Error: %s\n", fmt);
}

// Abre arquivo .geojson
char *geoJSONFile_read(const char* filename)
{
    FILE* fp;
    long file_size;
    static char* buffer = NULL;
    
    // Clean previous buffer
    if (buffer != NULL){
        free(buffer);
        buffer = NULL;
    }

    fp = fopen(filename, "r");
    if (fp == NULL){
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);

    buffer = malloc(file_size + 1);
    if (buffer == NULL){
        fclose(fp);
        return NULL;
    }

    rewind(fp);
    fread(buffer, file_size, 1, fp);
    fclose(fp);

    buffer[file_size] = '\0';
    return buffer;
}

void geoJSONFile_destroy(void){
    char* clean = geoJSONFile_read(NULL);
}

void help(int xcode)
{
    xcode = xcode ? xcode : 0;

    printf ("\n %s, version %s\n\n"
            "  usage:  %s [-hv -f file (stdin)] [file]\n\n"
            "  Reads a GeoJSON file containing points, writes a Voronoi Diagram\n"
            "  to stdout.\n\n"
            "    Options:\n\n"
            "      -f file    specifies filename to read.\n"
            "                 (note: file can be specified with or without -f option)\n"
            "      -h         display this help.\n"
            "      -v         display version information.\n\n",
            PACKAGE, VERSION, PACKAGE);
    exit (xcode);
}

char* processopts (int argc, char *argv[])
{
    static int opt;

    while ((opt = getopt(argc, argv, ":f:hv")) != -1) {
        switch (opt) {
            case 'f':
                return optarg;
                break;
            case 'h':       // help
                help (EXIT_SUCCESS);
            case 'v':       // show version information
                printf ("\n%s, version %s\n", PACKAGE, VERSION);
                exit (EXIT_SUCCESS);
            default :
                fprintf (stderr, "\nError: invalid or missing option.\n");
                help (EXIT_FAILURE);
        }
    }
    if (argc > optind) 
        return optarg;
    else
        help (EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    char *geoJSON;
    char *filename;

    filename = processopts(argc, argv);
    geoJSON = geoJSONFile_read(filename);

    initGEOS(notice, error);

    // Convert GeoJSON in GEOS geometry
    GEOSGeoJSONReader *reader = GEOSGeoJSONReader_create();
    GEOSGeometry *geom = GEOSGeoJSONReader_readGeometry(reader, geoJSON);
    if (!geom) {
        printf("Failed to read geometry from GeoJSON file.\n");
        GEOSGeoJSONReader_destroy(reader);
        finishGEOS();
        return 1;
    }
    
    // Extract points from Feature(s) or complain
    const char geomType = GEOSGeomTypeId(geom);
    GEOSGeometry *points;

    if (geomType == GEOS_POINT || geomType == GEOS_MULTIPOINT){
        puts("é ponto\n");
        points = GEOSGeom_clone(geom);
    }
    else if (geomType == GEOS_GEOMETRYCOLLECTION) {
        // Extract points from geometry collection
        points = NULL;
        int ngeoms = GEOSGetNumGeometries(geom);
        GEOSGeometry **geoms = malloc(ngeoms * sizeof(GEOSGeometry*));
        
        for (int i = 0; i < ngeoms; i++) {
            geoms[i] = GEOSGeom_clone(GEOSGetGeometryN(geom, i));
        }
        
        points = GEOSGeom_createCollection(GEOS_MULTIPOINT, geoms, ngeoms);
        free(geoms);
    }
    else {
        GEOSGeoJSONReader_destroy(reader);
        GEOSGeom_destroy(geom);
        geoJSONFile_destroy();
        finishGEOS();
        printf("Cant't create Voronoi diagram from %s based geometries.\n", GEOSGeomType(geom));
        return 0;
    }

    // Create envelope to use as clipping region (with some padding)
    GEOSGeometry *env = GEOSEnvelope(points);
    if (!env) {
        printf("Failed to create envelope.\n");
        GEOSGeom_destroy(points);
        GEOSGeom_destroy(geom);
        GEOSGeoJSONReader_destroy(reader);
        finishGEOS();
        return 1;
    }

    // Compute Voronoi diagram
    GEOSGeometry *voronoi = GEOSVoronoiDiagram(points, env, 0.0, 0);
    if (!voronoi) {
        printf("Failed to compute Voronoi diagram.\n");
        GEOSGeom_destroy(env);
        GEOSGeom_destroy(points);
        GEOSGeom_destroy(geom);
        GEOSGeoJSONReader_destroy(reader);
        finishGEOS();
        return 1;
    }   

    // Write it out as GeoJSON
    GEOSGeoJSONWriter* json_writer = GEOSGeoJSONWriter_create();
    int indentation = 4;
    unsigned char* json = GEOSGeoJSONWriter_writeGeometry(json_writer, voronoi, indentation);
    printf("GeoJSON Voronoi Diagram:\n%s\n\n", json);
    
    // Clean up allocated objects and context
    GEOSGeoJSONWriter_destroy(json_writer);
    GEOSGeoJSONReader_destroy(reader);
    GEOSGeom_destroy(geom);
    geoJSONFile_destroy();
    finishGEOS();
    return 0;
}