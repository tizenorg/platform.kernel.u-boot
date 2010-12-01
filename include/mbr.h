/*
 * Copyright (C) 2010 Samsung Electrnoics
 */

void set_mbr_table(unsigned int start_addr, int parts,
		unsigned int *blocks, unsigned int *part_offset);
int get_mbr_table(unsigned int *part_offset);
