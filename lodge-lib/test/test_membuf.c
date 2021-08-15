// membuf test
#if 0
	{
	float my_floats[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
	membuf_t buf = membuf_wrap(my_floats);

	{
		for(int i = 0, count = membuf_max_count(buf); i < count; i++) {
			printf("my_floats[%d] = %f\n", i, *(float*)membuf_get(buf, i));
		}
	}

	{
		size_t max_count = membuf_max_count(buf);
		//membuf_delete(my_floats_buf, 1, 2, &tail_index);
		membuf_delete_swap_tail(buf, 1, &max_count);

		for(int i = 0, count = membuf_max_count(buf); i < count; i++) {
			printf("my_floats[%d] = %f\n", i, *(float*)membuf_get(buf, i));
		}
	}

	{
		float fill = 0.0f;
		membuf_fill(buf, &fill, sizeof(fill));
		for(int i = 0, count = membuf_max_count(buf); i < count; i++) {
			printf("my_floats[%d] = %f\n", i, *(float*)membuf_get(buf, i));
		}
	}

	{
		size_t max_count = 0;
		float f;
		f = 11.0f; membuf_append(buf, &f, sizeof(f), &max_count);
		f = 12.0f; membuf_append(buf, &f, sizeof(f), &max_count);
		f = 13.0f; membuf_append(buf, &f, sizeof(f), &max_count);
		f = 14.0f; membuf_append(buf, &f, sizeof(f), &max_count);
		f = 15.0f; membuf_append(buf, &f, sizeof(f), &max_count);

		for(int i = 0, count = membuf_max_count(buf); i < count; i++) {
			printf("my_floats[%d] = %f\n", i, *(float*)membuf_get(buf, i));
		}
	}
	}
#endif