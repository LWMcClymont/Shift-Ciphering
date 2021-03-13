
__kernel void task (__global int16 *input1,
					__global int *n,
					__global int *d,
					__global int *output) {


		int id = get_global_id(0);

		int16 chunk = input1[id];
		
		int16 mask = chunk < 65 || chunk > 90;

		chunk = chunk + *n;
		chunk = chunk - 65;
		chunk = chunk % 26;
		chunk = chunk + 26;
		chunk = chunk % 26;
		chunk = chunk + 65;

		int16 result = select(chunk, input1[id], mask);

		vstore16(result, id, output);
		
}
