/*
 * $Id$
 *
 * Copyright (C) 2003 ETC s.r.o.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * Written by Matan Ziv-Av <matan@svgalib.org>, 2003.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BSBIT_INPUT     1
#define BSBIT_OUTPUT    2
#define BSBIT_CONTROL   3
#define BSBIT_INTERNAL  4

char dev_name[64]="";
int IR_l=0;
int BR_l=0;
int idcode[64];
int id_num=0;
struct bsbit {
	int bit;
	char name[32];
	int type;
//	signal *signal;
	int safe;               /* safe value */
	int control;            /* -1 for none */
	int control_value;
	int control_state;
} *bs_bits;
char insts[100][2][64];
int inst_num=0;
char pins[2048][16], pinsp[2048][1024];
int pins_num=-1;

char line[1024], pline[1024];
int mode;


int endline(void) {
	char att[64], tmp[64], id[32];
	int i, j;
	
//	if(pline[0])printf("%s\n", pline);
	
	if(!strncmp(pline, "entity ", 7)) {
		if(dev_name[0]==0) sscanf(pline, "entity %s", dev_name);
		mode=0;
	} else 
	if(!strncmp(pline, "attribute ", 10)) {
		i=sscanf(pline, "attribute %s", att);
		if(i!=1) return -1;
		if(!strncmp(att, "INSTRUCTION_LENGTH", 18)) {
			i=sscanf(pline, "attribute INSTRUCTION_LENGTH of %s : entity is %i ;", tmp, &j);
			if(i!=2) return -1;
			IR_l=j;
		} else
		if(!strncmp(att, "BOUNDARY_LENGTH", 15)){
			i=sscanf(pline, "attribute BOUNDARY_LENGTH of %s : entity is %i ;", tmp, &j);
			if(i!=2) return -1;
			BR_l=j;
			bs_bits=malloc(BR_l * sizeof(struct bsbit));
			for(i=0;i<BR_l;i++)bs_bits[i].bit=-1;
		} else
		if(!strncmp(att, "IDCODE_REGISTER", 15)) {
			i=sscanf(pline, "attribute IDCODE_REGISTER of %s : entity is %n", tmp, &j);
			if(i!=1) return -1;
			memmove(pline, pline+j, strlen(pline+j)+1);
			mode=4;
		} else 
		if(!strncmp(att, "BOUNDARY_REGISTER", 17)) {
			i=sscanf(pline, "attribute BOUNDARY_REGISTER of %s : entity is %n", tmp, &j);
			if(i!=1) return -1;
			memmove(pline, pline+j, strlen(pline+j)+1);
			mode=5;
		} else
		if(!strncmp(att, "INSTRUCTION_OPCODE", 18)) {
			i=sscanf(pline, "attribute INSTRUCTION_OPCODE of %s : entity is %n", tmp, &j);
			if(i!=1) return -1;
			memmove(pline, pline+j, strlen(pline+j)+1);
			mode=6;
		} else
		{
			mode=0;
		}
	} else
#if 0
	if(!strncmp(pline, "constant ", 9)) {
		if(strstr(pline, "PIN_MAP_STRING")) {
			for(i=1;i<strlen(pline);i++) {
				if(pline[i]=='=' && pline[i-1]==':') break;
			}
			memmove(pline, pline+i+1, strlen(pline+i+1)+1);
			mode=7;
		}
	} else
#else
	if(!strncmp(pline, "port", 4)) {
		for(i=1;i<strlen(pline);i++) {
			if(pline[i]=='(') break;
		}
		memmove(pline, pline+i+1, strlen(pline+i+1)+1);
		mode=8;
	} else 
#endif
	if(!strncmp(pline, "use", 3)) {
		mode=0;
	} else				
	if(!strncmp(pline, "end ", 4)) {
		return 1;
	} 
	{
		int ctl, dis, bit;
		char name[32], cell[16], type[16], safe[4], ds[4], sep; 
		int j,k;

		switch(mode) {
			case 1:
				i=sscanf(pline, "\"%16c\" &", id+4);
				if(i!=1)return -1;
				mode=2;
				break;
			case 2:
				i=sscanf(pline, "\"%11c\" &", id+20);
				if(i!=1)return -1;
				mode=3;
				break;
			case 3:
				i=sscanf(pline, "\"%1c\" &", id+31);
				if(i!=1){
					i=sscanf(pline, "\"%1c,\" ;", id+31);
					mode=0;
					return -1;
				} else mode=4;
				idcode[id_num]=strtoul(id, NULL, 2);
				id_num++;
				break;
			case 4:
				i=sscanf(pline, "\"%4c\" &", id);
				if(i!=1)return -1;
				mode=1;
				break;
			case 5:
				for(i=0;i<strlen(pline);i++)
					if((pline[i]==',')||(pline[i]=='"')) pline[i]=' ';
				for(i=0;i<strlen(pline);i++)
					if((pline[i]=='(')) {
						pline[i]=' ';
						break;
					}
				for(i=strlen(pline)-1;i>0;i--)
					if((pline[i]==')')) {
						pline[i]=' ';
						break;
					}
				
				i=sscanf(pline, " %i %s %s %s %s %i %i %s %c", 
					&bit, cell, name, type, safe, &ctl, &dis, ds, &sep);
				if(i!=9) {
					safe[1]=0;
					i=sscanf(pline, " %i %s %s %s %s %c",
						&bit, cell, name, type, safe, &sep);
					if(i!=6) return -i;
					if(!strncasecmp(type, "internal", 8)) {
						bs_bits[bit].type=BSBIT_INTERNAL;
					} else 
					if(!strncasecmp(type, "input", 5)) {
						bs_bits[bit].type=BSBIT_INPUT;
					} else 
					if(!strncasecmp(type, "OUTPUT", 6)) {
						bs_bits[bit].type=BSBIT_OUTPUT;
					} else 
					if(!strncasecmp(type, "control", 7)) {
						bs_bits[bit].type=BSBIT_CONTROL;
					} else bs_bits[bit].type=-1;
					bs_bits[bit].control = -1;
					bs_bits[bit].control_value = -1;
				} else {
					bs_bits[bit].type = BSBIT_OUTPUT;
					bs_bits[bit].control = ctl;
					bs_bits[bit].control_value = dis;
				}
				bs_bits[bit].bit = bit;
				strcpy(bs_bits[bit].name, name);
				bs_bits[bit].safe = safe[0]=='0'? 0 : 1;
				bs_bits[bit].control_state = -1;

				if(sep=='&') mode=5; else mode=0;
				break;
			case 6:
                for(i=0;i<strlen(pline);i++)
					if((pline[i]==',')||(pline[i]=='"')||(pline[i]=='(')||(pline[i]==')')) 
						pline[i]=' ';
				i=sscanf(pline, "%s %s", att, tmp);
				j=1;
				if(!strlen(att)==IR_l) {
					j=0;
					for(k=0;k<IR_l;k++)
						if((att[k]!='1')&&(att[k]!='0')&&(att[k]!='x')&&(att[k]!='X'))j=1;
				}
				if(pline[strlen(pline)-1]==';')mode=0;
				if(j && (i!=2)) {
					return -1;
				}
				if(j) {
					strcpy(insts[inst_num][0], att);
					strcpy(insts[inst_num][1], tmp);
					inst_num++;
				}
				break;
			case 7:
				j=0;
				for(i=0;i<strlen(pline);i++)if(pline[i]==':'){j=1; break;};
                for(i=0;i<strlen(pline);i++)
					if((pline[i]==',')||(pline[i]=='"')||(pline[i]=='(')||(pline[i]==')'))
						pline[i]=' ';
				if(j) {
					pins_num++;
					i=-1;
					sscanf(pline, " %s %n", pins[pins_num], &i);
					pins[pins_num][strlen(pins[pins_num])-1]='\0';
					if(i==-1)return -1;
					strcpy(pinsp[pins_num], pline+i);
					k=pinsp[pins_num][strlen(pinsp[pins_num])-1];
					pinsp[pins_num][strlen(pinsp[pins_num])-1]='\0';
				} else {
					strcat(pinsp[pins_num], " ");
					strcat(pinsp[pins_num], pline);
					k=pinsp[pins_num][strlen(pinsp[pins_num])-1];
					pinsp[pins_num][strlen(pinsp[pins_num])-1]='\0';
				}
				if(k=='&') mode=7; else mode=0;
				break;
			case 8:
				for(i=0;i<strlen(pline);i++)if(pline[i]==':')pline[i]=' ';
				pins_num++;
				sscanf(pline, "%s %s %n\n", pins[pins_num], tmp, &i);
				if(!strncmp(pline+i, "bit_vector", 10)) {
					int f,t;
					strcpy(tmp, pins[pins_num]);
					sscanf(pline+i, "bit_vector ( %i to %i )", &f, &t);
					while (f<=t) {
						sprintf(pins[pins_num], "%s%i",tmp,f);
						f++;
						pins_num++;
					}
					pins_num--;
				};
				break;
						
			case 0:
			default:
				/* ignore */
				break;
		}
	}
	
	pline[0]=0;
 	return 0;	
}

int noparentheses(char *s){
	int l, i;

	l=strlen(s);
	for(i=0;i<l;i++){
		if((s[i]=='(')||(s[i]==')')) {
			int j;
			for(j=i;j<l;j++)s[j]=s[j+1];
			l--;
			i--;
		}
	}
	return 0;
}

int main(int argc, char *argv[]) {
	FILE *infile;
	int i;
	
	infile=stdin;
	pline[0]=0;

	while(1) {
		int b, j;
		char c;
		
		i=fscanf(infile, " %s", line);
		b=fgetc(infile);
		j=strlen(line);
		c=line[j-1];
		
		if(line[0]=='-' && line[1]=='-') {
			if(b!='\n') fgets(line, 1023, infile);
		} else {
			if(pline[0])strcat(pline, " ");
			strcat(pline, line);
			if((c==';')||(c=='&')) {
				if((i=endline())){
					fprintf(stderr, "Return value is %i\n",i);
					if(1) {
						int j;
						/* pin */
						for(j=0;j<pins_num;j++) {
							noparentheses(pins[j]);
							printf("pin %s\n", pins[j]);
//							printf("pin %s %s\n", pins[j], pinsp[j]);
						}
						printf("\n");
					
						/* register */
						printf("register\tBSR\t%i\n",BR_l);
						printf("register\tBR\t1\n");
						printf("register\tDIR\t32\n\n");

						/* instruction */
						printf("instruction length %i\n\n",IR_l);

						for(j=0;j<inst_num;j++) {
							if(!strcasecmp(insts[j][0], "sample"))
								printf("instruction SAMPLE/PRELOAD %s BSR\n", insts[j][1]);
							if(!strcasecmp(insts[j][0], "extest"))
								printf("instruction EXTEST %s BSR\n", insts[j][1]);
							if(!strcasecmp(insts[j][0], "idcode"))
								printf("instruction IDCODE %s DIR\n", insts[j][1]);
							if(!strcasecmp(insts[j][0], "bypass"))
								printf("instruction BYPASS %s BR\n", insts[j][1]);
						}
						printf("\n");
						
						/* bit */
						for(j=BR_l-1;j>=0;j--) if(bs_bits[j].bit==j) {
							char types[6]="OXIOCO"; /* why internal is O */

							noparentheses(bs_bits[j].name);
							printf("bit %i %c %i %s",j, types[bs_bits[j].type+1],
									bs_bits[j].safe, bs_bits[j].name);
							if(bs_bits[j].control>=0) {
								printf(" %i %i Z\n", bs_bits[j].control, bs_bits[j].control_value);
							} else printf("\n");
						}
					}
					exit(0);
				}
			}
		}


	}
}
