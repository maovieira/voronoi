#include <stdio.h>
#include <stdarg.h>

#include <geos_c.h>


//  GEOS requires two message handlers to return
//  error and  notice message to the calling program.
static void
geos_msg_handler(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

int main()
{
    // Send notice and error messages to terminal
    initGEOS(geos_msg_handler, geos_msg_handler);

    const char* linestring = "LINESTRING(0 0 1, 1 1 1, 2 1 2)";
    // const char* point = "POINT(1 1)";

    // read WKT into geometry object
    GEOSWKTReader* reader = GEOSWKTReader_create();
    GEOSGeometry* geom_a = GEOSWKTReader_read(reader, linestring);

    // convert result to WKT
    GEOSWKTWriter* writer = GEOSWKTWriter_create();
    char* wkt = GEOSWKTWriter_write(writer, geom_a);
    printf("WKT Geometry:\n%s\n\n", wkt);
    GEOSWKTWriter_destroy(writer);

    // Write it out as GeoJSON
    GEOSGeoJSONWriter* json_writer = GEOSGeoJSONWriter_create();
    // Generate the JSON, with an indentation of 4
    int indentation = 4;
    unsigned char* json = GEOSGeoJSONWriter_writeGeometry(json_writer, geom_a, indentation);
    printf("GeoJSON Geometry:\n%s\n\n", json);
    GEOSGeoJSONWriter_destroy(json_writer);

    // Clean up allocated objects
    GEOSWKTReader_destroy(reader);
    GEOSGeom_destroy(geom_a);
    GEOSFree(wkt);

    // Clean up the global context
    finishGEOS();
    return 0;
}