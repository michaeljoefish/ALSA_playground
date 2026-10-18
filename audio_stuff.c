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

	// Don't forget to cleanup. First free the identifiers, and then the list.
	snd_ctl_elem_list_free_space(elem_list);
	snd_ctl_elem_list_free(elem_list);

	// Close control interface.
	snd_ctl_close(ctl_handle);


	return 0;
}

