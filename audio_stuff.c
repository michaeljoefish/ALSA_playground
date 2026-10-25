#include <stdio.h>
#include <alsa/asoundlib.h>
#include <alsa/control.h>
#include <alsa/control_plugin.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

static char *device = "hw:0,0";            /* playback device */
//unsigned char buffer[64*1024];              /* some random data */

void extract_values(snd_ctl_elem_type_t value_type, snd_ctl_elem_value_t* element_value, snd_ctl_elem_info_t* element_info, unsigned int value_count){
	switch (value_type){
		case(SND_CTL_ELEM_TYPE_BOOLEAN):
			for (unsigned int i=0; i<value_count; i++){
				const char* bool_str;
				if (snd_ctl_elem_value_get_boolean(element_value, i)) bool_str = "True";
				else bool_str = "False";
				printf("Boolean %u: %s\t", i, bool_str);
			}
			printf("\n");
			break;
		case(SND_CTL_ELEM_TYPE_INTEGER):
			for (unsigned int i=0; i<value_count; i++){
				printf("Integer %u: %ld\t", i, snd_ctl_elem_value_get_integer(element_value, i));
			}
			printf("Min: %ld\tMax: %ld\tStep: %ld\n", snd_ctl_elem_info_get_min(element_info),
					snd_ctl_elem_info_get_max(element_info), snd_ctl_elem_info_get_step(element_info));
			break;
		case(SND_CTL_ELEM_TYPE_INTEGER64):
			for (unsigned int i=0; i<value_count; i++){
				printf("Integer64 %u: %lld\t", i, snd_ctl_elem_value_get_integer64(element_value, i));
			}
			printf("Min64: %lld\tMax64: %lld\tStep64: %lld\n", snd_ctl_elem_info_get_min64(element_info),
					snd_ctl_elem_info_get_max64(element_info), snd_ctl_elem_info_get_step64(element_info));
			break;
		case(SND_CTL_ELEM_TYPE_BYTES):
			printf("Printing up to 4 bytes of this value.");
			unsigned char* bytes = (unsigned char*)snd_ctl_elem_value_get_bytes(element_value);
			for (unsigned int i=0; i<value_count && i<4; i++){
				printf("Byte %u: %hhu\t", i, bytes[i]);
			}
			printf("\n");
			break;
		case(SND_CTL_ELEM_TYPE_ENUMERATED):
			for (unsigned int i=0; i<value_count; i++){
				printf("Enum %u: %u\t", i, snd_ctl_elem_value_get_enumerated(element_value, i));
			}
			printf("\n");
			break;
		case(SND_CTL_ELEM_TYPE_IEC958):
			printf("There's IEC958 data in here. Don't know how to use it.\n");
			break;
		default:
			printf("INVALID TYPE\n");

	}
}

int examine_ctl_data(){
	void** hints = NULL;
	int result = snd_device_name_hint(-1, "ctl", &hints);
	if (result != 0) {printf("Failed to get hints. Error Code: %d\n", result); return result;}
	int i = 0;
	char* hint_name = NULL;
	char* hint_desc = NULL;
	char* hint_iod = NULL;
	while (hints[i] != NULL){
		printf("%d\n", i);
		hint_name = snd_device_name_get_hint(hints[i], "NAME");
		hint_desc = snd_device_name_get_hint(hints[i], "DESC");
		hint_iod = snd_device_name_get_hint(hints[i], "IOD");
		if (hint_name != NULL) printf("NAME: %s\n", hint_name);
		if (hint_desc != NULL) printf("DESC: %s\n", hint_desc);
		if (hint_iod != NULL) printf("IOD: %s\n", hint_iod);
		i++;
	}

	printf("\n\n");

	snd_device_name_free_hint(hints);

	snd_ctl_t* ctl_handle = NULL;
	result = snd_ctl_open(&ctl_handle, "hw:CARD=AudioPCI", SND_CTL_NONBLOCK);

	if (result != 0){printf("Failed to open control interface. Error code: %d\n", result); return result;}

	// Create and initialize element list.
	snd_ctl_elem_list_t* elem_list = NULL;
	snd_ctl_elem_list_malloc(&elem_list);

	// Now fill in what's basically meta data so we can know how much space to allocate for element identifiers.
	snd_ctl_elem_list(ctl_handle, elem_list);
	unsigned int num_entries = snd_ctl_elem_list_get_count(elem_list);

	// Now allocate enough space for all element identifiers. Still working with the element list.
	snd_ctl_elem_list_alloc_space(elem_list, num_entries);

	// Now actually get all the indentifiers and put them in the allocated space.
	snd_ctl_elem_list(ctl_handle, elem_list);	// Yes, this is the same as above.
	
	// Fun stuff happens here.
	// TODO: Fun stuff.
	// NOTE: Hasn't been fun so far.
	
	// Allocate an element id on the stack.
	snd_ctl_elem_id_t* element_id;
	snd_ctl_elem_id_alloca(&element_id);	// ALWAYS MAKE SURE YOU ALLOCATE SPACE FOR THE OPAQUE TYPES. CAN BE STACK OR HEAP.
	
	// Allocate an element info struct on the stack.
	snd_ctl_elem_info_t* element_info;
	snd_ctl_elem_info_alloca(&element_info);
	
	// Allocate an invalid element value on the stack.
	snd_ctl_elem_value_t* element_value;
	snd_ctl_elem_value_alloca(&element_value);
	
	for (unsigned int i = 0; i<num_entries; i++){
		printf("Element #%u\n", i);
		// Getting element identifier from element i in elem_list.
		snd_ctl_elem_list_get_id(elem_list, i, element_id);
		// Getting identifiers from the identifier.
		unsigned int numid = snd_ctl_elem_id_get_numid(element_id);
		snd_ctl_elem_iface_t interface = snd_ctl_elem_id_get_interface(element_id);
		const char* interface_name = snd_ctl_elem_iface_name(interface);
		unsigned int device = snd_ctl_elem_id_get_device(element_id);
		unsigned int subdevice = snd_ctl_elem_id_get_subdevice(element_id);
		const char* name = snd_ctl_elem_id_get_name(element_id);
		unsigned int index = snd_ctl_elem_id_get_index(element_id);
		printf("NumID: %u\tIface_name: %s\tDevice: %u\tSubDevice: %u\tName: %s\tIndex: %u\n",
				numid, interface_name, device, subdevice, name, index);
		

		// Getting element info from element list.
		snd_ctl_elem_info_set_id(element_info, element_id);
		result = snd_ctl_elem_info(ctl_handle, element_info);
		if (result != 0){printf("Failed to get element info. Error code: %d\n", result); return result;}
		unsigned int value_count = snd_ctl_elem_info_get_count(element_info);
		unsigned int info_device = snd_ctl_elem_info_get_device(element_info);
		unsigned int info_index = snd_ctl_elem_info_get_index(element_info);
		// Find the type of the element.
		snd_ctl_elem_type_t element_type = snd_ctl_elem_info_get_type(element_info);
		// Get type name in ascii, just for testing purposes.
		const char* type_name = snd_ctl_elem_type_name(element_type);
		printf("Value_type: %s\tValue_count: %u\tInfo_Device: %u\tInfo_Index: %u\n", type_name, value_count, info_device, info_index);
		

		// Now get info on all members (values) of each element.
		// Init ID data for value.
		snd_ctl_elem_value_set_id(element_value, element_id);
		// Now we should read element data into element value.
		snd_ctl_elem_read(ctl_handle, element_value);
		// Printing out some values for testing. TODO: Update this to include getting type-related data.
		/*
		unsigned int value_numid = snd_ctl_elem_value_get_numid(element_value);
		snd_ctl_elem_iface_t value_iface = snd_ctl_elem_value_get_interface(element_value);
		const char* value_iface_name = snd_ctl_elem_iface_name(value_iface);
		unsigned int value_device = snd_ctl_elem_value_get_device(element_value);
		unsigned int value_subdevice = snd_ctl_elem_value_get_subdevice(element_value);
		const char* value_name = snd_ctl_elem_value_get_name(element_value);
		unsigned int value_index = snd_ctl_elem_value_get_index(element_value);
		printf("Val_NumID: %u\tVal_iface_name: %s\tVal_device: %u\tVal_subdevice: %u\tVal_name: %s\tVal_idx: %u\n",
				value_numid, value_iface_name, value_device, value_subdevice, value_name, value_index);

		if (element_type == SND_CTL_ELEM_TYPE_INTEGER){
		}
		*/
		extract_values(element_type, element_value, element_info, value_count);
		printf("\n");
		
	}

	

	// Don't forget to cleanup. First free the identifiers, and then the list.
	snd_ctl_elem_list_free_space(elem_list);
	snd_ctl_elem_list_free(elem_list);

	// Close control interface.
	snd_ctl_close(ctl_handle);


	return 0;
}

int pcm_sound_min_example(){
	int fd;
	struct stat file_stats;
	int err;
	if ((fd = open("./samples/M1F1-uint8WE-AFsp.wav", O_RDONLY)) < 0){printf("Failed to open file.\n");return 1;}
	if (fstat(fd, &file_stats) < 0){perror("fstat pcm_sound_min_example"); return 1;}
	size_t file_size = file_stats.st_size;
	unsigned char* buffer = malloc(sizeof(unsigned char*) * file_size);
	read(fd, (void*)buffer, file_size);
	unsigned int fmt_chunk_size = *(unsigned int*)(buffer + 16) + 8;
	printf("fmt chunk size = %u\n", fmt_chunk_size);
	short num_channels = *(short*)(buffer + 22);
	printf("Num channels = %hd\n", num_channels);
	int sample_rate = *(int*)(buffer + 24);
	printf("Sample rate = %d\n", sample_rate);
	short bit_depth = *(short*)(buffer + 34);
	printf("Bit depth = %hd\n", bit_depth);
	unsigned int block_id = *(unsigned int*)(buffer + 12 + fmt_chunk_size);
	unsigned int block_id_offset = 12 + fmt_chunk_size;
	printf("Block_id = %x\tBlock_id_offset = %u\n", block_id, block_id_offset);
	unsigned int data_offset = block_id_offset + 4;
	unsigned int data_size = *(unsigned int*)(buffer + data_offset);
	printf("Data size (bytes) = %u\n", data_size);

	/*for (unsigned int i = data_offset; i<(data_offset + data_size); i+=2){
		unsigned char temp = buffer[i];
		buffer[i] |= buffer[i+1];
		buffer[i+1] |= temp;
	}*/

	close(fd);
	unsigned int i;
	snd_pcm_t *handle;
	snd_pcm_sframes_t frames;

	//for (i = 0; i < sizeof(buffer); i++) {
	//	buffer[i] = (unsigned char)round((127.5*0.125) * sin(2*M_PI * (440.0/48000.0)*i) + 127.5);
	//}
	//buffer[i] = (unsigned char)((i >> 8) & 0x0FF);
	//buffer[i] = random() & 0xff;

	if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
	printf("Playback open error: %s\n", snd_strerror(err));
	exit(EXIT_FAILURE);
	}
	if ((err = snd_pcm_set_params(handle,
		      SND_PCM_FORMAT_U8,
		      SND_PCM_ACCESS_RW_INTERLEAVED,
		      num_channels,
		      sample_rate,
		      1,
		      500000)) < 0) {   /* 0.5sec */
	printf("Playback open error: %s\n", snd_strerror(err));
	exit(EXIT_FAILURE);
	}

	for (i = 0; i < 1; i++) {
	frames = snd_pcm_writei(handle, buffer+data_offset, data_size);
	if (frames < 0)
	    frames = snd_pcm_recover(handle, frames, 0);
	if (frames < 0) {
	    printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
	    break;
	}
	if (frames > 0 && frames < (long)data_size)
	    printf("Short write (expected %li, wrote %li)\n", (long)data_size, frames);
	}

	/* pass the remaining samples, otherwise they're dropped in close */
	err = snd_pcm_drain(handle);
	if (err < 0)
	printf("snd_pcm_drain failed: %s\n", snd_strerror(err));
	snd_pcm_close(handle);
	free(buffer);
	return 0;
}

int main(){
	int result;
	//int result = examine_ctl_data();
	result = pcm_sound_min_example();
	return result;
}

