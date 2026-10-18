#include <stdio.h>
#include <alsa/asoundlib.h>
#include <alsa/control.h>
#include <alsa/control_plugin.h>

int main(){
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
	for (unsigned int i = 0; i<num_entries; i++){
		printf("Element #%u\n", i);
		// Getting element identifier from element i in elem_list.
		snd_ctl_elem_id_t* element_id;
		snd_ctl_elem_list_get_id(elem_list, i, element_id);
		// Getting identifiers from the identifier.
		unsigned int numid = snd_ctl_elem_id_get_numid(element_id);
		snd_ctl_elem_iface_t interface = snd_ctl_elem_id_get_interface(element_id);
		const char* interface_name = snd_ctl_elem_iface_name(interface);
		unsigned int device = snd_ctl_elem_id_get_device(element_id);
		unsigned int subdevice = snd_ctl_elem_id_get_subdevice(element_id);
		const char* name = snd_ctl_elem_id_get_name(element_id);
		unsigned int index = snd_ctl_elem_id_get_index(element_id);

		printf("NumID: %u, Iface_name: %s, Device: %u, SubDevice: %u, Name: %s, Index: %u\n",
				numid, interface_name, device, subdevice, name, index);
		// Getting element info from element list.

		snd_ctl_elem_info_t* element_info;
		snd_ctl_elem_info_alloca(&element_info);
		snd_ctl_elem_info_set_id(element_info, element_id);
		result = snd_ctl_elem_info(ctl_handle, element_info);
		if (result != 0){printf("Failed to get element info. Error code: %d\n", result); return result;}
		unsigned int value_count = snd_ctl_elem_info_get_count(element_info);
		unsigned int info_device = snd_ctl_elem_info_get_device(element_info);
		unsigned int info_index = snd_ctl_elem_info_get_index(element_info);
		printf("Value_count: %u, Info_Device: %u, Info_Index: %u\n\n", value_count, info_device, info_index);
	}

	/*snd_ctl_elem_info_t* element_info;
	printf("Uno\n");
	result = snd_ctl_elem_info(ctl_handle, element_info);
	if (result != 0){printf("Failed to get element info. Error code: %d\n", result); return result;}
	printf("Dos\n");*/
	

	// Don't forget to cleanup. First free the identifiers, and then the list.
	snd_ctl_elem_list_free_space(elem_list);
	snd_ctl_elem_list_free(elem_list);

	// Close control interface.
	snd_ctl_close(ctl_handle);


	return 0;
}

