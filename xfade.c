/* amp.c

   Free software by Richard W.E. Furse. Do with as you will. No
   warranty.

   This LADSPA plugin provides simple mono and stereo amplifiers.

   This file has poor memory protection. Failures during malloc() will
   not recover nicely. */

/*****************************************************************************/

#include <stdlib.h>
#include <string.h>

/*****************************************************************************/

#include "ladspa.h"

/*****************************************************************************/

/* The port numbers for the plugin: */

#define XFADE_CONTROL 0
#define XFADE_CHANNEL 1
#define XFADE_INPUT1  2
#define XFADE_OUTPUT1 3
#define XFADE_INPUT2  4
#define XFADE_OUTPUT2 5

/*****************************************************************************/

/* The structure used to hold port connection information and state
   (actually gain controls require no further state). */

typedef struct {

  LADSPA_Data * m_pfControlValue;
  LADSPA_Data * m_pfChannelValue;
  LADSPA_Data * m_pfInputBuffer1;
  LADSPA_Data * m_pfOutputBuffer1;
  LADSPA_Data * m_pfInputBuffer2;  
  LADSPA_Data * m_pfOutputBuffer2; 

} Crossfade;

/*****************************************************************************/

/* Construct a new plugin instance. */
LADSPA_Handle 
instantiateCrossfade(const LADSPA_Descriptor * Descriptor,
		     unsigned long             SampleRate) {
  return malloc(sizeof(Crossfade));
}

/*****************************************************************************/

/* Connect a port to a data location. */
void 
connectPortToCrossfade(LADSPA_Handle Instance,
		       unsigned long Port,
		       LADSPA_Data * DataLocation) {

  Crossfade * psCrossfade;

  psCrossfade = (Crossfade *)Instance;
  switch (Port) {
  case XFADE_CONTROL:
    psCrossfade->m_pfControlValue = DataLocation;
    break;
  case XFADE_CHANNEL:
    psCrossfade->m_pfChannelValue = DataLocation;
    break;
  case XFADE_INPUT1:
    psCrossfade->m_pfInputBuffer1 = DataLocation;
    break;
  case XFADE_OUTPUT1:
    psCrossfade->m_pfOutputBuffer1 = DataLocation;
    break;
  case XFADE_INPUT2:
    psCrossfade->m_pfInputBuffer2 = DataLocation;
    break;
  case XFADE_OUTPUT2:    
    psCrossfade->m_pfOutputBuffer2 = DataLocation;
    break;
  }
}

/*****************************************************************************/

void 
runCrossfade(LADSPA_Handle Instance,
		   unsigned long SampleCount) {
  
  LADSPA_Data * pfInput;
  LADSPA_Data * pfOutput;
  LADSPA_Data fGain;
  LADSPA_Data fChannel;
  Crossfade * psCrossfade;
  unsigned long lSampleIndex;

  psCrossfade = (Crossfade *)Instance;

  fGain = *(psCrossfade->m_pfControlValue);
  fChannel = *(psCrossfade->m_pfChannelValue);

  pfInput = psCrossfade->m_pfInputBuffer1;
  pfOutput = psCrossfade->m_pfOutputBuffer1;
  for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) 
    *(pfOutput++) = *(pfInput++) * (fChannel != 0.0f ? fGain : (1.0f - fGain));

  pfInput = psCrossfade->m_pfInputBuffer2;
  pfOutput = psCrossfade->m_pfOutputBuffer2;
  for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) 
    *(pfOutput++) = *(pfInput++) * (fChannel != 0.0f ? fGain : (1.0f - fGain));
}

/*****************************************************************************/

/* Throw away a simple delay line. */
void 
cleanupCrossfade(LADSPA_Handle Instance) {
  free(Instance);
}

/*****************************************************************************/

LADSPA_Descriptor * g_psStereoDescriptor = NULL;

/*****************************************************************************/

/* _init() is called automatically when the plugin library is first
   loaded. */
void 
_init() {

  char ** pcPortNames;
  LADSPA_PortDescriptor * piPortDescriptors;
  LADSPA_PortRangeHint * psPortRangeHints;

  g_psStereoDescriptor 
    = (LADSPA_Descriptor *)malloc(sizeof(LADSPA_Descriptor));

  
  if (g_psStereoDescriptor) {
    
    g_psStereoDescriptor->UniqueID
      = 998;
    g_psStereoDescriptor->Label
      = strdup("crossfader");
    g_psStereoDescriptor->Properties
      = LADSPA_PROPERTY_HARD_RT_CAPABLE;
    g_psStereoDescriptor->Name 
      = strdup("2-channel Crossfader");
    g_psStereoDescriptor->Maker
      = strdup("Vesa Kivimäki");
    g_psStereoDescriptor->Copyright
      = strdup("LGPLv2");
    g_psStereoDescriptor->PortCount
      = 6;
    piPortDescriptors
      = (LADSPA_PortDescriptor *)calloc(6, sizeof(LADSPA_PortDescriptor));
    g_psStereoDescriptor->PortDescriptors
      = (const LADSPA_PortDescriptor *)piPortDescriptors;
    piPortDescriptors[XFADE_CONTROL]
      = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    piPortDescriptors[XFADE_CHANNEL]
      = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;    
    piPortDescriptors[XFADE_INPUT1]
      = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    piPortDescriptors[XFADE_OUTPUT1]
      = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
    piPortDescriptors[XFADE_INPUT2]
      = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    piPortDescriptors[XFADE_OUTPUT2]
      = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
    pcPortNames
      = (char **)calloc(6, sizeof(char *));
    g_psStereoDescriptor->PortNames 
      = (const char **)pcPortNames;
    pcPortNames[XFADE_CONTROL]
      = strdup("Crossfade");
    pcPortNames[XFADE_CHANNEL]
      = strdup("Channel");
    pcPortNames[XFADE_INPUT1]
      = strdup("Input (Left)");
    pcPortNames[XFADE_OUTPUT1]
      = strdup("Output (Left)");
    pcPortNames[XFADE_INPUT2]
      = strdup("Input (Right)");
    pcPortNames[XFADE_OUTPUT2]
      = strdup("Output (Right)");
      
    psPortRangeHints = ((LADSPA_PortRangeHint *)
			calloc(6, sizeof(LADSPA_PortRangeHint)));
    g_psStereoDescriptor->PortRangeHints
      = (const LADSPA_PortRangeHint *)psPortRangeHints;
      
    psPortRangeHints[XFADE_CONTROL].HintDescriptor
      =(  LADSPA_HINT_BOUNDED_BELOW 
      |   LADSPA_HINT_LOGARITHMIC
      |   LADSPA_HINT_DEFAULT_1
      |   LADSPA_HINT_BOUNDED_ABOVE
      );      
    psPortRangeHints[XFADE_CONTROL].LowerBound 
      = 0;
    psPortRangeHints[XFADE_CONTROL].UpperBound
      = 1;
      
    psPortRangeHints[XFADE_CHANNEL].HintDescriptor
      =(  LADSPA_HINT_BOUNDED_BELOW
      |   LADSPA_HINT_BOUNDED_ABOVE
      |   LADSPA_HINT_TOGGLED
      |   LADSPA_HINT_DEFAULT_0   
      );
    psPortRangeHints[XFADE_CHANNEL].LowerBound = 0;
    psPortRangeHints[XFADE_CHANNEL].UpperBound = 1;
      
    psPortRangeHints[XFADE_INPUT1].HintDescriptor
      = 0;
    psPortRangeHints[XFADE_OUTPUT1].HintDescriptor
      = 0;
    psPortRangeHints[XFADE_INPUT2].HintDescriptor
      = 0;
    psPortRangeHints[XFADE_OUTPUT2].HintDescriptor
      = 0;
    g_psStereoDescriptor->instantiate 
      = instantiateCrossfade;
    g_psStereoDescriptor->connect_port 
      = connectPortToCrossfade;
    g_psStereoDescriptor->activate
      = NULL;
    g_psStereoDescriptor->run
      = runCrossfade;
    g_psStereoDescriptor->run_adding
      = NULL;
    g_psStereoDescriptor->set_run_adding_gain
      = NULL;
    g_psStereoDescriptor->deactivate
      = NULL;
    g_psStereoDescriptor->cleanup
      = cleanupCrossfade;
  }
}

/*****************************************************************************/

void
deleteDescriptor(LADSPA_Descriptor * psDescriptor) {
  unsigned long lIndex;
  if (psDescriptor) {
    free((char *)psDescriptor->Label);
    free((char *)psDescriptor->Name);
    free((char *)psDescriptor->Maker);
    free((char *)psDescriptor->Copyright);
    free((LADSPA_PortDescriptor *)psDescriptor->PortDescriptors);
    for (lIndex = 0; lIndex < psDescriptor->PortCount; lIndex++)
      free((char *)(psDescriptor->PortNames[lIndex]));
    free((char **)psDescriptor->PortNames);
    free((LADSPA_PortRangeHint *)psDescriptor->PortRangeHints);
    free(psDescriptor);
  }
}

/*****************************************************************************/

/* _fini() is called automatically when the library is unloaded. */
void
_fini() {
  deleteDescriptor(g_psStereoDescriptor);
}

/*****************************************************************************/

/* Return a descriptor of the requested plugin type. There are two
   plugin types available in this library (mono and stereo). */
const LADSPA_Descriptor * 
ladspa_descriptor(unsigned long Index) {
  /* Return the requested descriptor or null if the index is out of
     range. */
  switch (Index) {
  case 0:
    return g_psStereoDescriptor;
  default:
    return NULL;
  }
}

/*****************************************************************************/

/* EOF */
