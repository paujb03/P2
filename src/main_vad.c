#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sndfile.h>

#include "vad.h"
#include "vad_docopt.h"

#define DEBUG_VAD 0x1

int main(int argc, char *argv[]) {
  int verbose = 0; /* To show internal state of vad: verbose = DEBUG_VAD; */

  SNDFILE *sndfile_in, *sndfile_out = 0;
  SF_INFO sf_info;
  FILE *vadfile;
  int n_read = 0, i;

  VAD_DATA *vad_data;
  VAD_STATE state, last_state, undefined_state;

  float *buffer, *buffer_zeros;
  int frame_size;         /* in samples */
  float frame_duration;   /* in seconds */
  unsigned int t, last_t; /* in frames */

  char	*input_wav, *output_vad, *output_wav;

  //resultado de analizar main_vad con docopt
  DocoptArgs args = docopt(argc, argv, /* help */ 1, /* version */ "2.0");

  verbose    = args.verbose ? DEBUG_VAD : 0;
  input_wav  = args.input_wav;
  output_vad = args.output_vad;
  output_wav = args.output_wav;
 
  // Conversión de cadenas de carácteres a números flotantes utilizando atof
  //float alpha1 = atof(args.alpha1);
  //float alpha2 = atof(args.alpha2);
  float alpha1 = 5.12;
  float alpha2 = 6.52;


  // Verificación de la existencia de los archivos de entrada y salida
  if (input_wav == 0 || output_vad == 0) {
    fprintf(stderr, "%s\n", args.usage_pattern);
    return -1;
  }

  /* Apertura del archivo de audio de entrada */
  if ((sndfile_in = sf_open(input_wav, SFM_READ, &sf_info)) == 0) {
    fprintf(stderr, "Error opening input file %s (%s)\n", input_wav, strerror(errno));
    return -1;
  }

  // Verificación de que el archivo de entrada sea mono
  if (sf_info.channels != 1) {
    fprintf(stderr, "Error: the input file has to be mono: %s\n", input_wav);
    return -2;
  }

  /* Apertura del archivo VAD */
  if ((vadfile = fopen(output_vad, "wt")) == 0) {
    fprintf(stderr, "Error opening output vad file %s (%s)\n", output_vad, strerror(errno));
    return -1;
  }

  /* Apertura del archivo de salida WAVE si se ha proporcionado un nombre de archivo */
  if (output_wav != 0) {
    if ((sndfile_out = sf_open(output_wav, SFM_WRITE, &sf_info)) == 0) {
      fprintf(stderr, "Error opening output wave file %s (%s)\n", output_wav, strerror(errno));
      return -1;
    }
  }


//   
  vad_data = vad_open(sf_info.samplerate);
  /* Allocate memory for buffers */
  frame_size   = vad_frame_size(vad_data);
  buffer       = (float *) malloc(frame_size * sizeof(float));
  buffer_zeros = (float *) malloc(frame_size * sizeof(float));
  for (i=0; i< frame_size; ++i) buffer_zeros[i] = 0.0F;

  frame_duration = (float) frame_size/ (float) sf_info.samplerate;
  
  last_state = ST_SILENCE; 
  undefined_state = ST_SILENCE; 

  for (t = last_t = 0; ; t++) { /* For each frame ... */
    /* End loop when file has finished (or there is an error) */
    if  ((n_read = sf_read_float(sndfile_in, buffer, frame_size)) != frame_size) 
    break;

    if (sndfile_out != 0) {
      /* TODO: copy all the samples into sndfile_out */
      sf_writef_float(sndfile_out,buffer,frame_size);
    }

    state = vad(vad_data, buffer, alpha1, alpha2); 

    if (verbose & DEBUG_VAD) vad_show_state(vad_data, stdout);

    /* TODO: print only SILENCE and VOICE labels */
    /* As it is, it prints UNDEF segments but is should be merge to the proper value */
    if (state != last_state) {
      if(state == ST_UNDEF) {
        undefined_state = last_state; //guardo l'últim estat vàlid
        // printf("%d, ",state);
      }
      if (t != last_t) {
        if ((state != undefined_state) && (last_state == ST_UNDEF)) { // si canvies d'estat
          if (undefined_state == ST_SILENCE) {
            if (sndfile_out != 0) {
              sf_seek(sndfile_out,last_t*frame_size, SEEK_SET); //nos movemos al punto inicial
            }
          }
          last_state = state;
        } else if (state != ST_UNDEF) { // tornes al mateix estat
          fprintf(vadfile, "%.5f\t%.5f\t%s\n", last_t * frame_duration, t * frame_duration, state2str(last_state));
          last_state = state;
          last_t = t;
        }
      }
    }
    // ampliació: poso a 0 el silenci
    if (state==ST_SILENCE && sndfile_out != 0){
      sf_seek(sndfile_out, -frame_size, SEEK_CUR); // nos posicionamos en el frame que queremos silenciar
      sf_write_float(sndfile_out, buffer_zeros, frame_size); // escribimos todo ceros
      last_state = state;
    }
  }

  state = vad_close(vad_data);
  /* TODO: what do you want to print, for last frames? */
  if (t != last_t)
    fprintf(vadfile, "%.5f\t%.5f\t%s\n", last_t * frame_duration, t * frame_duration + n_read / (float) sf_info.samplerate, state2str(state));

  /* clean up: free memory, close open files */
  free(buffer);
  free(buffer_zeros);
  sf_close(sndfile_in);
  fclose(vadfile);
  if (sndfile_out) sf_close(sndfile_out);
  return 0;
}