/* gcc `pkg-config --cflags --libs librsvg-2.0 cairo-pdf` -o svg2pdf svg2pdf.c
 *
 *
 * Public Domain Mark 1.0
 * No Copyright
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>

#include <cairo-pdf.h>

#define FAIL(msg)                           \
    do { fprintf (stderr, "FAIL: %s\n", msg); exit (-1); } while (0)

#define PIXELS_PER_POINT 1

cairo_status_t stream_cairo_write (void *closure, const unsigned char *data, unsigned int length)
{
    FILE *bufferout = closure;
    if (bufferout == NULL)
    FAIL ("");
    fwrite(data, 1, length, bufferout);

    return CAIRO_STATUS_SUCCESS;
}

long svg_to_pdf(const unsigned char* svg_content, long svg_length, char **pdf_content, float dpi)
{
    GError *error = NULL;
    RsvgHandle *handle;
    RsvgDimensionData dim;
    float width, height;
    cairo_surface_t *surface;
    cairo_t *cr;
    cairo_status_t status;

    FILE *fptmp;
    long pdf_size;

    g_type_init ();

    rsvg_set_default_dpi (dpi);

    handle = rsvg_handle_new_from_data(svg_content, svg_length, &error);

    if (error != NULL)
    FAIL (error->message);

    rsvg_handle_get_dimensions (handle, &dim);
    width = dim.width;
    height = dim.height;

    fptmp = tmpfile();
    if (fptmp == NULL)
    FAIL ("Unable to create temporary file");
    surface = cairo_pdf_surface_create_for_stream(stream_cairo_write, fptmp, width, height);
    cr = cairo_create (surface);

    rsvg_handle_render_cairo (handle, cr);

    status = cairo_status (cr);
    if (status)
    FAIL (cairo_status_to_string (status));

    cairo_destroy (cr);
    cairo_surface_destroy (surface);

    fseek(fptmp, 0, SEEK_END);
    pdf_size = ftell(fptmp);
    rewind(fptmp);
    (*pdf_content) = (char*) malloc(sizeof(char)*pdf_size);
    if ((*pdf_content) == NULL)
    FAIL ("");
    fread(*pdf_content, 1, pdf_size, fptmp);
    fclose(fptmp);

    return pdf_size;
}

int main (int argc, char *argv[])
{
    FILE *fp;
    long lSize;
    unsigned char *buffer;
    char *bufferout;
    long outSize;
    FILE *fpout;

    if ((argc < 3) | (argc > 4))
    FAIL ("usage: svg2pdf input_file.svg output_file.pdf [dpi]");

    const char *filename = argv[1];
    const char *output_filename = argv[2];
    float dpi = 0.0f;
    if (argc > 3) dpi = atof(argv[3]);
    if (dpi > 0.0f) dpi = 5184.0f / dpi;


    fp = fopen(filename, "r");
    if (fp == NULL)
    FAIL ("Couldn't open input file");
    fseek(fp, 0, SEEK_END);
    lSize = ftell(fp);
    rewind(fp);
    buffer = (unsigned char *) malloc(sizeof(unsigned char)*lSize);
    if (buffer == NULL)
    FAIL ("");
    fread(buffer, 1, lSize, fp);

    outSize = svg_to_pdf(buffer, lSize, &bufferout, dpi);

    fpout = fopen(output_filename, "wb");
    if (fpout == NULL)
    FAIL ("Unable to save file.");
    fwrite(bufferout, 1, outSize, fpout);

    fclose(fp);
    fclose(fpout);
    free(bufferout);
    free(buffer);

    return 0;
}
