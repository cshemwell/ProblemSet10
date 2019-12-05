# ProblemSet10 README.md

I was pretty lost when I started to attempt this problem set. Even the tutor wasn't sure how PortAudio stuff worked before the Thanksgiving break, but I visited him again on Wednesday and he was able to lead me in the right direction. I think my biggest weakness in C is basic values. Pointers, typecasting, and value types still catch me up, which is the reason I can't quite wrap my head around these ProblemSets by myself. The bigger concepts like PortAudio and libsndfile all make 100% sense, it's just the smaller syntaxes that I don't fully understand.
Anyways, I figured out 10.1 by creating an i-- for loop:  ->

    for(int i = 0; i < -1 * gSfInfo.frames; i--){
        sf_seek(gInFile, i, SEEK_END);
        sf_read_float(gInFile, outBuffer + (-i), 1);

I initially thought it should be outBuffer[-i], but the tutor told me about the other syntax which seemed to work.

10.2 was a lot of copying/pasting for me. Copy/pasting the SoundFile struct and the openInputSndFile function.
To actually load "sine.aif" into the Wavetable.table, I just used the openInputSndFile prototype followed by sf_read_float.
Lines 165-168 are where I altered the createWavtable function to write the audio to table within the Wavetable struct.
 
