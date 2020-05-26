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
    else{
        // clear buffer
        *stream->buffer = '\0';
        // restart position at 0;
        stream->pos = 0;
        // readjust size to 0
        stream->actual_size = 0;
        return 0;
    }
}

/**
 * Gets the next character (an unsigned char) from the specified stream and advances the position indicator for the stream.
 * @param stream
 * @return  Returns the character currently pointed by the internal file position indicator of the specified stream.
 * If the stream is at the end-of-file when called, the function returns EOF
 */
int fgetc(FILE *stream) {
    // purge buffer if in write mode
    if (stream->lastop == 'w') {
        fpurge(stream);
    }
    if (stream->actual_size == 0) {
        // clear buffer
        fpurge(stream);
        // fill the buffer using read system call
        int n = read(stream->fd, stream->buffer, stream->size);
        // set lastop to read mode
        stream->lastop = 'r';
        // adjust actual size to new refill size
        stream->actual_size = n;
        // if new actual_size is equal to zero, it is empty, and we reached eof
        if(stream->actual_size <= 0 ) {
            stream->eof = true;
            return EOF;
        }
        // set position pointer to the beginning buffer
        stream->pos = 0;
    }
        // if there is more to read in the buffer
    else {
        // increment position
        stream->pos++;
    }
    // read one char from buffer
    int ch = stream->buffer[stream->pos];
    stream->actual_size -= sizeof((char) ch);
    // return char
    return ch;
}
/**
 * Reads characters from stream and stores them as a C string into str until (num-1) characters
 * have been read or either a newline or the end-of-file is reached, whichever happens first.
 * A newline character makes fgets stop reading
 * A terminating null character is automatically appended after the characters copied to str.
 * @param str Pointer to an array of chars where the string read is copied.
 * @param size Maximum number of characters to be copied into str
 * @param stream Pointer to a FILE object that identifies an input stream.
 * @return On success, the function returns str.
 */
char *fgets(char *str, int size, FILE *stream)
{
    // purge buffer if in write mode
    if (stream->lastop == 'w') {
        fpurge(stream);
    }
    // set lastop to read mode
    stream->lastop = 'r';
    int c;
    char *newString = str;
    // get max bytes or upto a newline
    for (int i = size ; size > 0; size--) {
        //get char and break if EOF
        if ((c = fgetc (stream)) == EOF){
            stream->eof = true;
            break;
        }
        //copy c into newString
        *newString++ = c;
        //break if c is a newline
        if (c == '\n')
            break;
    }
    *newString = 0;
    //if same as str return null
    if (newString == str) return NULL;
    //else return the newString with data
    return newString;

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

    // purge buffer if in write mode
    if (stream->lastop == 'w') {
        fpurge(stream);
    }
    size_t bytesRead = 0;
    size_t bufferSize = nmemb * size;
    // set lastop to read mode
    stream->lastop = 'r';
    int c;
    unsigned char* buffPtr = static_cast<unsigned char*>(ptr);
    // while number of bytes read is less than size to read
    while(bytesRead < bufferSize){
        //get char one by one using fgetc
        //if EOF break
        if ((c = fgetc (stream)) == EOF){
            stream->eof = true;
            break;
        }
        //increase bytes read
        bytesRead++;
        // copy character into the userspace buffer
        *buffPtr++ = c;
    }
    int itemsRead = bytesRead / size;
    return itemsRead;

}

/**
 * Forces a write of all user-space buffered data for the given output
 * If the stream argument is NULL, fflush() flushes all open output streams.
 * @param stream
 * @return Upon successful completion 0 is returned.  Otherwise, EOF is returned
 */
int fflush(FILE *stream)
{
    if (stream == NULL) {
        return EOF;
    }
    else {
        // write on to device using the write system call
        int bytesWritten = write(stream->fd, stream->buffer, stream->pos);
        // change lastop to write
        stream->lastop = 'w';
        // if nothing was written, return EOF
        if (bytesWritten == -1) {
            return EOF;
        }
        //stream->actual_size = bytesWritten; IF THIS DOESN'T WORK, RETURN TO COMMENT
        stream->actual_size = 0;
        // restart position pointer to 0
        stream->pos = 0;
        return 0;
    }
}
/**
 * Writes the C string pointed by str to the stream.
 * @param c C string with the content to be written to stream.
 * @param stream Pointer to a FILE object that identifies an output stream.
 * @return On success, a non-negative value is returned.
 * On error, the function returns EOF and sets the error indicator (ferror).
 */
int fputc(int c, FILE *stream)
{
    // purge buffer if in read mode
    if (stream->lastop == 'r') {
        fpurge(stream);
    }
    // if the position pointer reached the end of the buffer
    if (stream->pos == stream->size) {
        // flush the buffer
        int n = fflush(stream);
        if (n == EOF) {
            stream->eof = true;
            return EOF;
        }
    }
    // place character into buffer at current position
    stream->buffer[stream->pos] = c;
    // increment position counter
    stream->pos++;
    return c;
}
/**
 *Writes an array of nmemb elements, each one with a size of size bytes,
 * from the block of memory pointed by ptr to the current position in the stream.
 * @param ptr Pointer to the array of elements to be written, converted to a const void*.
 * @param size Size in bytes of each element to be written.
 * @param nmemb Number of elements, each one with a size of size bytes.
 * @param stream Pointer to a FILE object that specifies an output stream.
 * @return The total number of elements successfully written is returned.
 */
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{

    int byteCounter = 0;
    const unsigned char* buffPtr = static_cast<const unsigned char*>(ptr);
    // while number of bytes to write is less than size to write
    while (byteCounter < (size * nmemb)) {
        char ch = *buffPtr;
        if (ch == EOF) {
            stream->eof = true;
            return EOF;
        }
        int n = fputc(ch, stream);
        // increment byteCounter
        byteCounter += sizeof((char) n);
        buffPtr++;
    }
    int itemsWritten = byteCounter / size;
    return itemsWritten;
}

/**
 *Writes the C string pointed by str to the stream.
 * @param str C string with the content to be written to stream.
 * @param stream Pointer to a FILE object that identifies an output stream.
 * @return On success, a non-negative value is returned.
   On error, the function returns EOF and sets the error indicator
 */
int fputs(const char *str, FILE *stream)
{
    if (stream->lastop == 'r') {
        fpurge(stream);
    }
    if (str == NULL) {
        return EOF;
    } else {
        //write the string using the fwrite function
        int bytesWritten = fwrite(str, strlen(str), 1, stream);
        if (bytesWritten <= 0) {
            return EOF;
        }
        return bytesWritten;
    }
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
    if (whence != SEEK_SET && whence != SEEK_END && whence!=SEEK_CUR)
        return -1;
    stream->eof=false;
    //flush the stream if in write mode

    fflush(stream);
    int n = lseek(stream->fd, offset, whence);
    if (n <= 0) return -1;
    else return 0;
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
    fflush(stream);
    // close stream
    int stat = close(stream->fd);
    // deallocate stream
    delete stream;
    return stat;
}
