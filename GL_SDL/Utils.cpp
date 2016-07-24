//
//  Utils.cpp
//  GL_SDL
//
//  Created by George Sealy on 24/07/16.
//  Copyright © 2016 MixBit. All rights reserved.
//

struct Timer {
    double ticksToSeconds;
    
    uint64_t startTick;
    
    Timer() {
        mach_timebase_info_data_t timebase_info;
        mach_timebase_info(&timebase_info);
        
        //        const uint64_t NANOS_PER_MSEC = 1000000ULL;
        ticksToSeconds = ((double)timebase_info.denom / (double)timebase_info.numer) / 1000000000.0;
        
        startTick = mach_absolute_time();
    }
    
    double seconds() {
        uint64_t nowTick = mach_absolute_time();
        return (nowTick - startTick) * ticksToSeconds;
    }
};


char *fileToCharArray(const char *file)
{
    FILE *fptr;
    long length;
    char *buf;
    
    fptr = fopen(file, "rb"); /* Open file for reading */
    if (!fptr) /* Return NULL on failure */
    return NULL;
    
    fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
    length = ftell(fptr); /* Find out how many bytes into the file we are */
    buf = (char*)malloc(length+1); /* Allocate a buffer for the entire length of the file and a null terminator */
    fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
    fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
    fclose(fptr); /* Close the file */
    buf[length] = 0; /* Null terminator */
    
    return buf; /* Return the buffer */
}
