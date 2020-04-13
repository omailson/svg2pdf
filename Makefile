ALL=svg2pdf

CFLAGS=`pkg-config --cflags librsvg-2.0 cairo-pdf` -Wall -Wpointer-arith -Wstrict-prototypes -Wnested-externs -fno-strict-aliasing
LDFLAGS=`pkg-config --libs librsvg-2.0 cairo-pdf`

all: $(ALL)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	rm -f $(ALL) *.o
