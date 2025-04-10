#include <math.h>
#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <stdio.h>
#include "pav_analysis.h"
#include "vad.h"

const float FRAME_TIME = 10.0F; /* in ms. */

/* 
 * As the output state is only ST_VOICE, ST_SILENCE, or ST_UNDEF,
 * only this labels are needed. You need to add all labels, in case
 * you want to print the internal state in string format
 */

const char *state_str[] = {
  "UNDEF", "S", "MS", "V"// "MV", "INIT"
};

const char *state2str(VAD_STATE st) {
  return state_str[st];
}

/* Define a datatype with interesting features */
typedef struct {
  float zcr;
  float p;
  float am;
} Features;

/* 
 * TODO: Delete and use your own features!
 */

Features compute_features(const float *x, int N) {
  /*
   * Input: x[i] : i=0 .... N-1 
   * Ouput: computed features
   */
  /* 
   * DELETE and include a call to your own functions
   *
   * For the moment, compute random value between 0 and 1 
   */
  Features feat;
  feat.p = compute_power (x,N);
  feat.zcr = compute_zcr(x,N,16000);
  feat.am = compute_am(x,N);
  return feat;
}

/* 
 * TODO: Init the values of vad_data
 */

VAD_DATA * vad_open(float rate) {
  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state = ST_INIT;
  vad_data->sampling_rate = rate;
  vad_data->aux= 0;                                          
  vad_data->N = 0;                                     
  vad_data->frame_length = rate * FRAME_TIME * 1e-3;                                                                
  vad_data->k0 = 0;                                          
  return vad_data;
}

VAD_STATE vad_close(VAD_DATA *vad_data) {
  /* 
   * TODO: decide what to do with the last undecided frames
   */

  VAD_STATE state;
  if(vad_data->state == ST_SILENCE || vad_data->state == ST_VOICE){
    state = vad_data->state;
  } else {
    state = ST_SILENCE;
  }

  free(vad_data);
  return state;
}

unsigned int vad_frame_size(VAD_DATA *vad_data) {
  return vad_data->frame_length;
}

/* 
 * TODO: Implement the Voice Activity Detection 
 * using a Finite State Automata
 */

VAD_STATE vad(VAD_DATA *vad_data, float *x, float alpha1, float alpha2) {

  /* 
   * TODO: You can change this, using your own features,
   * program finite state automaton, define conditions, etc.
   */

  Features f = compute_features(x, vad_data->frame_length);
  vad_data->last_feature = f.p; /* save feature, in case you want to show */

  switch (vad_data->state) {
    case ST_INIT:
      vad_data->N++;
      vad_data->k0=10*log10(vad_data->k0+pow(10,(f.p/10))/vad_data->N);
      vad_data->state = ST_SILENCE;
      //printf("test st_init");
    break;

    case ST_SILENCE:
      if (f.p > vad_data->k0 + alpha1 || f.zcr > alpha2) { // los sonidos no sonoros pueden diferenciarse del silencio con el zcr 
        vad_data->state = ST_MAYBE_VOICE;
      } 
    break;

    case ST_VOICE:
      if (f.p < vad_data->k0 + alpha1) {
        vad_data->state = ST_MAYBE_SILENCE;
      } 
    break;
    // usamos la misma variable aux de contador para los dos estados maybe ya que no pueden compartirse, 
    // puesto que no se puede pasar de un maybe voice a un maybe silence
    case ST_MAYBE_VOICE:
      if(vad_data->aux >= 90) { // a partir de 90 frames saliendo maybe voice suponemos que era una falsa alarma
        vad_data->aux = 0;
        vad_data->state = ST_SILENCE;

      } else if (f.p > vad_data-> k0 + alpha1 + alpha2) { 
        vad_data->aux = 0;
        vad_data->state = ST_VOICE;
      } else {
        vad_data->aux++;
      }
    break;

    case ST_MAYBE_SILENCE:
      if(((f.p < vad_data->k0 + alpha1) && vad_data->aux >= 8)) { // a partir de 8 frames saliendo maybe silence ponemos silence
        vad_data->aux = 0;
        vad_data->state = ST_SILENCE;

      } else if (f.p > vad_data->k0 + alpha1 + alpha2) {
        vad_data->aux = 0;
        vad_data->state = ST_VOICE;
      } else {
        vad_data->aux++;
      }
    break;

    case ST_UNDEF:
      break;
  }

  if (vad_data->state == ST_SILENCE || vad_data->state == ST_VOICE) {
    return vad_data->state;
  } else if (vad_data->state == ST_INIT) {
    return ST_SILENCE;
  } else {
    return ST_UNDEF;
  }

}


void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
  fprintf(out, "%d\t%f\n", vad_data->state, vad_data->last_feature);
}