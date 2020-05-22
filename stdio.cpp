#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
using namespace std;

char decimal[100];

int recursive_itoa(int arg)
{
    int div = arg / 10;
    int mod = arg % 10;
    int index = 0;
    if (div > 0)
    {
        index = recursive_itoa(div);
    }
    decimal[index] = mod + '0';
    return ++index;
}

char *itoa(const int arg)
{
    bzero(decimal, 100);
    int order = recursive_itoa(arg);
    char *new_decimal = new char[order + 1];
    bcopy(decimal, new_decimal, order + 1);
    return new_decimal;
}

int printf(const void *format, ...)
{
    va_list list;
    va_start(list, format);

    char *msg = (char *)format;
    char buf[1024];
    int nWritten = 0;

    int i = 0, j = 0, k = 0;
    while (msg[i] != '\0')
    {
        if (msg[i] == '%' && msg[i + 1] == 'd')
        {
            buf[j] = '\0';
            nWritten += write(1, buf, j);
            j = 0;
            i += 2;

            int int_val = va_arg(list, int);
            char *dec = itoa(abs(int_val));
            if (int_val < 0)
            {
                nWritten += write(1, "-", 1);
            }
            nWritten += write(1, dec, strlen(dec));
            delete dec;
        }
        else
        {
            buf[j++] = msg[i++];
        }
    }
    if (j > 0)
    {
        nWritten += write(1, buf, j);
    }
    va_end( list );
    return nWritten;
}

int setvbuf(FILE *stream, char *buf, int mode, size_t size)
{
    if (mode != _IONBF && mode != _IOLBF && mode != _IOFBF)
    {
        return -1;
    }
    stream->mode = mode;
    stream->pos = 0;
    if (stream->buffer != (char *)0 && stream->bufown == true)
    {
        delete stream->buffer;
    }

    switch ( mode )
    {
        case _IONBF:
            stream->buffer = (char *)0;
            stream->size = 0;
            stream->bufown = false;
            break;
        case _IOLBF:
        case _IOFBF:
            if (buf != (char *)0)
            {
                stream->buffer = buf;
                stream->size   = size;
                stream->bufown = false;
            }
            else
            {
                stream->buffer = new char[BUFSIZ];
                stream->size = BUFSIZ;
                stream->bufown = true;
            }
            break;
    }
    return 0;
}

void setbuf(FILE *stream, char *buf)
{
    setvbuf(stream, buf, ( buf != (char *)0 ) ? _IOFBF : _IONBF , BUFSIZ);
}

FILE *fopen(const char *path, const char *mode)
{
    FILE *stream = new FILE();
    setvbuf(stream, (char *)0, _IOFBF, BUFSIZ);

    // fopen( ) mode
    // r or rb = O_RDONLY
    // w or wb = O_WRONLY | O_CREAT | O_TRUNC
    // a or ab = O_WRONLY | O_CREAT | O_APPEND
    // r+ or rb+ or r+b = O_RDWR
    // w+ or wb+ or w+b = O_RDWR | O_CREAT | O_TRUNC
    // a+ or ab+ or a+b = O_RDWR | O_CREAT | O_APPEND

    switch(mode[0])
    {
        case 'r':
            if (mode[1] == '\0')            // r
            {
                stream->flag = O_RDONLY;
            }
            else if ( mode[1] == 'b' )
            {
                if (mode[2] == '\0')          // rb
                {
                    stream->flag = O_RDONLY;
                }
                else if (mode[2] == '+')      // rb+
                {
                    stream->flag = O_RDWR;
                }
            }
            else if (mode[1] == '+')        // r+  r+b
            {
                stream->flag = O_RDWR;
            }
            break;
        case 'w':
            if (mode[1] == '\0')            // w
            {
                stream->flag = O_WRONLY | O_CREAT | O_TRUNC;
            }
            else if (mode[1] == 'b')
            {
                if (mode[2] == '\0')          // wb
                {
                    stream->flag = O_WRONLY | O_CREAT | O_TRUNC;
                }
                else if (mode[2] == '+')      // wb+
                {
                    stream->flag = O_RDWR | O_CREAT | O_TRUNC;
                }
            }
            else if (mode[1] == '+')        // w+  w+b
            {
                stream->flag = O_RDWR | O_CREAT | O_TRUNC;
            }
            break;
        case 'a':
            if (mode[1] == '\0')            // a
            {
                stream->flag = O_WRONLY | O_CREAT | O_APPEND;
            }
            else if (mode[1] == 'b')
            {
                if (mode[2] == '\0')          // ab
                {
                    stream->flag = O_WRONLY | O_CREAT | O_APPEND;
                }
                else if (mode[2] == '+')      // ab+
                {
                    stream->flag = O_RDWR | O_CREAT | O_APPEND;
                }
            }
            else if (mode[1] == '+')        // a+  a+b
            {
                stream->flag = O_RDWR | O_CREAT | O_APPEND;
            }
            break;
    }

    mode_t open_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

    if ((stream->fd = open(path, stream->flag, open_mode)) == -1)
    {
        delete stream;
        printf("fopen failed\n");
        stream = NULL;
    }

    return stream;
}

int feof(FILE *stream)
{
    return stream->eof == true;
}

int fpurge(FILE *stream)
{
    if(stream == NULL) return EOF;
    else
        stream->pos = 0;
    stream->buffer = NULL;
    return 0;
}
/**
 * Forces a write of all user-space buffered data for the given output
 * If the stream argument is NULL, fflush() flushes all open output streams.
 * @param stream
 * @return Upon successful completion 0 is returned.  Otherwise, EOF is returned
 */
int fflush(FILE *stream)
{
    int bytes_written;
    bytes_written = write(stream->fd,stream->buffer,stream->size);
    if(bytes_written==-1) return EOF;
    return bytes_written;
}

/**
 *
 * @param stream
 * @return  Returns the character currently pointed by the internal file position indicator of the specified stream.
 * The internal file position indicator is then advanced to the next character.
 * If the stream is at the end-of-file when called, the function returns EOF and
 * sets the end-of-file indicator for the stream (feof).
 * If a read error occurs, the function returns EOF and sets the error indicator for the stream (ferror).
 */
int fgetc(FILE *stream) {
    // if buffer was used in write mode previously, clear buffer
    if (stream->lastop == 'w') {
        printf("clearing stream buffer by calling fpurge...\n");
        fpurge(stream);
    }
    stream->lastop = 'r';

    if (stream->actual_size == 0 || stream->pos + 1 == stream->size) {
        stream->actual_size = read(stream->fd, stream->buffer, stream->size);
        stream->pos = 0;
    } else {
        stream->pos++;
    }
    if (stream->actual_size <= 0) {
        stream->eof = true;
        return EOF;
    }
    stream->actual_size--;
    return stream->buffer[stream->pos];
}
/**
 * Reads characters from stream and stores them as a C string into str until (num-1) characters
 * have been read or either a newline or the end-of-file is reached, whichever happens first.
 * A newline character makes fgets stop reading, but it is considered a valid character by the
 * function and included in the string copied to str.
 * A terminating null character is automatically appended after the characters copied to str.
 * @param str Pointer to an array of chars where the string read is copied.
 * @param size Maximum number of characters to be copied into str
 * @param stream Pointer to a FILE object that identifies an input stream.
 * @return On success, the function returns str.
 */
char *fgets(char *str, int size, FILE *stream)
{
   // printf ("position in the stream %d \n", stream->pos);
    if (stream->lastop == 'w') {
        printf("clearing stream buffer by calling fpurge...\n");
        fpurge(stream);
    }

    stream->lastop = 'r';
    int c;
    char *p;
    /* get max bytes or upto a newline */
    for (p = str, size--; size > 0; size--) {
        if ((c = fgetc (stream)) == EOF){
            stream->eof = true;
            break;
        }

        *p++ = c;
        if (c == '\n')
            break;
    }
    *p = 0;

    if (p == str || c == EOF)
        return NULL;
    return p;
}


/**
 * Reads an array of nmemb * size of bytes, from the stream and stores them in the block of memory specified by ptr.
 * The position indicator of the stream is advanced by the total amount of bytes read.
 * The total amount of bytes read if successful is (size*count).
 * @param ptr Pointer to a block of memory with a size of at least (size*count) bytes, converted to a void*.
 * @param size Size, in bytes, of each element to be read.
 * @param nmemb Number of elements, each one with a size of size bytes.
 * @param stream Pointer to a FILE object that specifies an input stream.
 * @return The total number of elements successfully read is returned.
 */
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    // if buffer was used in write mode previously, clear buffer
    if (stream->lastop == 'w') {
        printf("clearing stream buffer by calling fpurge...\n");
        fpurge(stream);
    }
    stream->lastop = 'r';
    size_t bufferSize = size * nmemb;
    size_t bytes_read;

    stream->actual_size = read(stream->fd, ptr, bufferSize);

    if (stream->actual_size <= 0) {
        stream->eof = true;
        return EOF;
    }
    bytes_read = stream->actual_size;
//
//    size_t bufferSize = size * nmemb;
//    char *p = static_cast<char *>(ptr);
//    size_t i;
//    for(i = 0; i < bufferSize; i++) {
//        int c = fgetc(stream);
//        if (c == EOF) break;
//        *p++ = c;
//    }
    return bytes_read / size;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    // comlete it
    return 0;
}

int fputc(int c, FILE *stream)
{
    //the write operations (fwrite, fputc, fputs)
    // need to add bytes to the buffer,
    // and when it's full / you've advanced to the end,
    // flush it to disk with write(), clear it out and start over
    // complete it
    char ch;
    ch = c;
    write(stream->fd, &ch, 1);
    return c;
}

int fputs(const char *str, FILE *stream)
{
    // complete it
    return 0;
}
/**
 * Sets the position indicator associated with the stream to a new position.
 * @param stream Pointer to a FILE object that identifies the stream.
 * @param offset Number of bytes to offset from origin.
 * @param whence Position used as reference for the offset.
 * @return If successful, the function returns zero.
   Otherwise, it returns non-zero value.
 */
int fseek(FILE *stream, long offset, int whence)
{
    //lseek returns the resulting offset location as measured in bytes from the beginning of the file.
    // On error, the value (off_t) -1 is returned and errno is set to indicate the error.
    stream->pos = lseek(stream->fd, offset, whence);
    return stream->pos;
}
/**
 * Closes the file associated with the stream and disassociates it.
 * @param stream Pointer to a FILE object that specifies the stream to be closed.
 * @return If the stream is successfully closed, a zero value is returned.
On failure, EOF is returned.
 */
int fclose(FILE *stream)
{
    // flush any unwritten output from the buffer to the file
    fpurge(stream);
    // close stream
    close(stream->fd);
    // deallocate stream
    delete stream;
    return 0;
}
