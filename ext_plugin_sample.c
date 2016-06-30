/*
 * Sample ALSA external plugin
 *
 * Copyright (c) 2016 by Jerome Anand <jeromelanand@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <alsa/asoundlib.h>
#include <alsa/pcm_external.h>
#include <stdio.h>

#define DUMMY
#define ENABLE_DUMP

/*ALGO file */
#ifdef DUMMY
#include <dummy_algo.h>
#endif

/*ALGO Type*/
enum algo_type { 
	None, 
#ifdef DUMMY
	Dummy,
#endif
} ;
 
typedef struct  {
        snd_pcm_extplug_t ext;
	/*ALGO type*/
	 int algo;
	/*Add ALGO specific data here*/
#ifdef ENABLE_DUMP
	int dump_enable;
	FILE* input_fptr;
	FILE* output_fptr;
	const char* input_fname;
	const char* output_fname;
#endif	
}myplug_info;

const unsigned int formats[2] = {
				SND_PCM_FORMAT_S32, 
				SND_PCM_FORMAT_S16
			      };
const unsigned int formate_length = 2;

#if defined(ENABLE_DUMP) || defined (DUMMY)
static inline void *area_addr(const snd_pcm_channel_area_t *area,
			      snd_pcm_uframes_t offset)
{        
	unsigned int bitofs = area->first + area->step * offset;
	return (char *) area->addr + bitofs / 8;
}
#endif	

/*Function my_own_init */
static int my_own_init(snd_pcm_extplug_t *ext)
{     
	myplug_info *myplug = (myplug_info *)ext;
     
	/*ALGO  init call*/
#ifdef DUMMY
	if(myplug->algo == Dummy)
		dummy_algo_init(); 
#endif

#ifdef ENABLE_DUMP
	if(myplug->dump_enable) {
		/*Check input-output file name is NULL*/
		if(myplug->input_fname == NULL || myplug->output_fname == NULL )
		{
			SNDERR("Use proper Input-Output file name");
			return -EINVAL;
		}

		/*Open input file*/
	 	myplug->input_fptr = fopen(myplug->input_fname,"wb"); 

	 	if(myplug->input_fptr == NULL) {
			SNDERR("File open failed for Input File");
			free(( char*)myplug->input_fname);
			return -ENOENT;
		}

		/*Open output file*/
		myplug->output_fptr = fopen(myplug->output_fname,"wb"); 
	
	 	if(myplug->output_fptr == NULL) {
			SNDERR("File open failed for output File");
			free(( char*)myplug->output_fname);
			return -ENOENT;
		}
	}
#endif
 	return 0; 
}

/*Function my_own_close*/
static int my_own_close(snd_pcm_extplug_t *ext)
{	
	myplug_info *myplug = (myplug_info *)ext;

	/*ALGO  close call*/
#ifdef DUMMY
	if(myplug->algo == Dummy)
		dummy_algo_close();
#endif

#ifdef ENABLE_DUMP
	if(myplug->dump_enable) {	
		/*Dump is not created if Input and Output Dump file names are same*/		
		if(   (myplug->input_fname != NULL && myplug->output_fname != NULL)) {
			printf("Dump File Created : \"Input file\" = %s and \"Output file\" = %s\n",
			        myplug->input_fname,myplug->output_fname);
		}

		/*Close input file*/
		if(myplug->input_fptr != NULL) 			
			fclose(myplug->input_fptr);

		/*Close outout File*/		
		if(myplug->output_fptr != NULL) 
			fclose(myplug->output_fptr);
		
		/*Delete memory allocated for File path*/
		if(myplug->input_fname != NULL)
			free(( char*) myplug->input_fname);

		if(myplug->output_fname != NULL)
			free(( char*) myplug->output_fname);
	}
#endif
	return 0;
}

/* Function my_own_transfer*/ 
static snd_pcm_sframes_t
my_own_transfer(snd_pcm_extplug_t *ext,
	       const snd_pcm_channel_area_t *dst_areas,
	       snd_pcm_uframes_t dst_offset,
	       const snd_pcm_channel_area_t *src_areas,
	       snd_pcm_uframes_t src_offset,
	       snd_pcm_uframes_t size)
{	
	myplug_info *myplug = (myplug_info *)ext;

	/*Dump audio data before Processing*/
#ifdef ENABLE_DUMP
	if(myplug->dump_enable) {
		short *src = area_addr(src_areas, src_offset);
		int size_bytes =  snd_pcm_frames_to_bytes (ext->pcm,size);

		fwrite(src,1,size_bytes,myplug->input_fptr);
	}
#endif

	/*ALGO  transfer call*/
#ifdef DUMMY
	if(myplug->algo == Dummy) {
		int size_bytes =  snd_pcm_frames_to_bytes (ext->pcm,size); 
		short *src = area_addr(src_areas, src_offset);
 		short *dst = area_addr(dst_areas, dst_offset);

		dummy_algo_transfer(dst,src,size_bytes);
	}
#endif	

	/*Dump audio data after Processing*/
#ifdef ENABLE_DUMP
	if(myplug->dump_enable) {
		short *dst = area_addr(dst_areas, dst_offset);
		int size_bytes =  snd_pcm_frames_to_bytes (ext->pcm,size);

		fwrite(dst,1,size_bytes,myplug->output_fptr);
	}
#endif
	return size;
}

static const snd_pcm_extplug_callback_t my_own_callback = {
	.init = my_own_init,
	.close = my_own_close,
	.transfer = my_own_transfer, /*This is a required callback*/
};

/* Display help */
void help() {
	printf( "Usage : \n"
		"	aplay -D myplug:[ ALGO=<algo name>,   | \n"
		"			  SLAVE=<salve name>, | \n"
	);

#ifdef ENABLE_DUMP
	printf( "			  EN_DUMP=<0 or 1>,   | \n"
	);
#endif

	printf( "			  HELP =<0 or 1>      ] <Audio File Name> \n"
		"\n"
		"Example : \n"
		" aplay -D myplug:ALGO=\\\"dummy\\\",SLAVE=\\\"hw:1,0,0\\\""
	);

#ifdef ENABLE_DUMP
	printf( ",EN_DUMP=1"
	);
#endif

	printf(
		" ~/s_48k_16.wav\n"
		"\n"
		"For help, \n"
		" aplay -D myplug:HELP=1 \n"
		"\n"
		"ALGO 	- algo name. Here available algo is \"dummy\" only.\n"
		"	  Default value is \"dummy.\" \n"
		"\n"
		"SLAVE 	- slave device name. Use aplay -l to find device.\n"
		"	  Default value is \"hw:1\" \n"
		"	  example: \\\"hw:1,0,0\\\" or \\\"plughw:1,0,0\\\" \n"
		"\n"
	);

#ifdef ENABLE_DUMP
	printf(
		"EN_DUMP - Enable audio dump. 0 -> disable and 1 -> enable. \n"
		"	  Default value is 0 -> disable. \n"
		"\n"
	);
#endif

	printf(
		"HELP  	- 1 to show Help. Default value is 0. \n"
		"\n"
		"The plugin support is available for S16 and S32 format as input, so \n"
		"if needed use \"plug\" layer appropriately to covert it. \n"
		"\n"
		"Example : \n"
		" aplay -Dplug:\\\'myplug:ALGO=\\\"dummy\\\"\\\' ~/s_48k_16.wav \n"
		"\n"
	);  
}

SND_PCM_PLUGIN_DEFINE_FUNC(myplug)
{
        snd_config_iterator_t i, next;
        snd_config_t *slave = NULL;
        myplug_info *myplug;
        int err; 
	int help_status = 0;

#ifdef ENABLE_DUMP
	const char *ifname = NULL, *ofname = NULL;
#endif
	myplug = calloc(1, sizeof(*myplug));
        if (myplug == NULL)
                return -ENOMEM;
	
	/*Defult initialization*/
	myplug->algo = None;
#ifdef ENABLE_DUMP
	myplug->dump_enable = 0;
	myplug->input_fptr = NULL;
	myplug->output_fptr = NULL;
	myplug->input_fname = NULL;
	myplug->output_fname = NULL;	
#endif
        snd_config_for_each(i, next, conf) {
                snd_config_t *n = snd_config_iterator_entry(i);
                const char *id;
                if (snd_config_get_id(n, &id) < 0)
                        continue;
                if (strcmp(id, "type") == 0 ) 
                        continue;
                if (strcmp(id, "slave") == 0) {
                        slave = n;
			continue;
                }
		if (strcmp(id, "help") == 0) {
			int val;
			val = snd_config_get_bool(n);
			if (val < 0) {
				SNDERR("Invalid value for %s. Expected boolean value.", id);
				return val;
			}
			help_status=val;			 
			continue;
		}
		if (strcmp(id, "algo") == 0) {
			const char* val;
			err = snd_config_get_string(n, &val);
			if (err < 0) {
				SNDERR("Invalid value for %s", id);
				return err;
			}
#ifdef DUMMY
			if ( strcmp (val,"dummy")==0 ){
			    myplug->algo = Dummy;			  	
			}
#endif
			continue;		
		} 
#ifdef ENABLE_DUMP
		if (strcmp(id, "dump_enable") == 0) {
			int val;
			val = snd_config_get_bool(n);
			if (val < 0) {
				SNDERR("Invalid value for %s. Expected boolean value.", id);
				return val;
			}
			myplug->dump_enable = val;
			continue;
		}
		if (strcmp(id, "input_fname") == 0) {
			err = snd_config_get_string(n, &ifname);
			if (err < 0) {
				SNDERR("Invalid value for %s", id);
				return err;
			}			
			continue;
		}
		if (strcmp(id, "output_fname") == 0) {
			err = snd_config_get_string(n, &ofname);
			if (err < 0) {
				SNDERR("Invalid value for %s", id);
				return err;
			}
			continue;
		}
#endif
                SNDERR("Unknown field %s", id);
                return -EINVAL;
        }	
        if (! slave) {
                SNDERR("No slave defined for myplug");
                return -EINVAL;
        }      	
	if ( myplug->algo == None ) {
		SNDERR("No algo defined for myplug or Invalid algo name.");
		 return -EINVAL;
        }
	if( help_status) {
 		help();
		return -EAGAIN;
	}

#ifdef ENABLE_DUMP
	if(myplug->dump_enable) {
		if(!ifname || strlen (ifname)== 0 ) {
			SNDERR("Input file name is not defined");
                	return -EINVAL;
		}
		if(!ofname || strlen (ofname)== 0 ) {
			SNDERR("Output file name is not defined");
                	return -EINVAL;
		}
		if(!strcmp(ifname,ofname) ){
			SNDERR("Input and Output Dump File names are same");
			return -EINVAL;
		}		
		myplug->input_fname = strdup (ifname);	 
		myplug->output_fname = strdup (ofname);
	}
#endif   
        myplug->ext.version = SND_PCM_EXTPLUG_VERSION;
        myplug->ext.name = "My Own Plugin";
        myplug->ext.callback = &my_own_callback;
        myplug->ext.private_data = myplug;
        	
	err = snd_pcm_extplug_create(&myplug->ext, name, root, slave, stream, mode);
        if (err < 0) {
                free(myplug);
                return err;
        }
	snd_pcm_extplug_set_param_list(&myplug->ext, SND_PCM_EXTPLUG_HW_FORMAT,
					formate_length, formats);
	snd_pcm_extplug_set_slave_param_list(&myplug->ext, SND_PCM_EXTPLUG_HW_FORMAT,
					formate_length, formats);	 	
        *pcmp = myplug->ext.pcm;	
        return 0;
}
SND_PCM_PLUGIN_SYMBOL(myplug);
