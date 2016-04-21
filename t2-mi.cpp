#include<iostream>
#include<fstream>
#include<string.h>
#include<stdlib.h>

using namespace std;

istream* inbuf;
ostream* outbuf;
int size;
char* t2packet;
int t2packetsize=0;
int t2packetpos=0;
bool active=false;
char* syncB;
bool first=true;
int pad=0;
char plpId=0;
int dnp=0;


void processt2(char* buf, int len, bool isStarting) {
	//std::cerr<< "len" << len << "starting:" << isStarting << std::endl;
	if(isStarting) {
		unsigned int offset=1;
		offset+=static_cast<unsigned char>(buf[0]);
		std::cerr << "offset " << offset << std::endl;
			fprintf(stderr, "packetspos:%i\n",t2packetpos);
		if(active) {
			if(offset>1) {// && (t2packetpos + offset -1 <= t2packetsize + 10 +  pad))  //4 bytes crc
				memcpy(&t2packet[t2packetpos],&buf[1],offset-1);
				//std::cerr << "HERE" << std::endl;
				//std::cerr << "pos: " << dec << t2packetpos << " len: " << (offset-1) << " size: " << t2packetsize << std::endl;
	//			if(t2packetpos+offset-1 >t2packetsize +4) std::cerr << "ERROR" << std::endl;
			}
			fprintf(stderr,"plpId:%02x\n",t2packet[7]);
			if(t2packet[7]==plpId) {
				fprintf(stderr,"%02x %02x<>\n",static_cast<unsigned char>(t2packet[9]),static_cast<unsigned char>(t2packet[10]));
				int syncd = (unsigned char) t2packet[16];
				syncd<<=8;
				syncd |= (unsigned char) (t2packet[17]);
				syncd >>= 3;
				int upl = (unsigned char) t2packet[13];
				upl <<=8;
				upl |= (unsigned char) (t2packet[14]);
				upl >>= 3;
				upl+=19;
				if(t2packet[9]&0x4) dnp=1;
				//if(upl>t2packetsize) upl=t2packetsize;
				//std::cerr << "UPL " << hex << upl << "  " <<  hex << t2packetsize << std::endl;
				if(syncd==0xFFFF ) {
					std::cerr << "SYNC" << std::endl;
					if(upl >19)
						outbuf->write(&t2packet[19],upl-19);
				}
				else {
					if(!first && syncd>0)
					outbuf->write(&t2packet[19],syncd-dnp);
					first=false;
					int j=19+syncd;
					for(; j< upl - 187; j+=(187+dnp)) {
						outbuf->write(syncB,1);
						outbuf->write(&t2packet[j],187);
					}
					if(j< upl )  {
						outbuf->write(syncB,1);
						outbuf->write(&t2packet[j],upl -j);
					}	
				}
			}
			active=false;
		}
		//std::cerr << "pointer " << hex << (int) (unsigned char)buf[13];
		//std::cerr << "pointer " << hex << (int) (unsigned char)buf[14];
		//std::cerr << "pointer " << hex << (int) (unsigned char)buf[15] << std::endl;
		fprintf(stderr, "art: ");
		for(int k=0;k<128;k++)
		fprintf(stderr, "%02x ",(unsigned char)buf[offset+k]);
		fprintf(stderr, "\n");
		
		if((buf[offset])==0x0) { //Baseband Frame
			t2packetsize= (unsigned char) buf[offset+4];
			t2packetsize<<=8;
			t2packetsize|= (unsigned char) (buf[offset+5]);
			//std::cerr << hex << (int) static_cast<unsigned char>(buf[offset+4]) << std::endl;
			//std::cerr << hex <<  (int) static_cast<unsigned char>(buf[offset+5]) << std::endl;
			//std::cerr << "p-size: " << hex << t2packetsize << std::endl;
			if(t2packetsize&0x07 !=0x00) {
			t2packetsize>>=3;
			t2packetsize+=6;
			pad=1;
			} else {
			t2packetsize>>=3;
			t2packetsize+=6;
			pad=0;
			}
			//if( (len - offset) >0 && t2packetsize >= 6) {
			if( (len - offset) >0) {
				memcpy(t2packet,&buf[offset],len-offset);
				t2packetpos=len-offset;
				active=true;
			}
		}
	} else if(active) {// && (t2packetpos+len <= t2packetsize + 10 + pad)) 
		//std::cerr << "pos: " << dec << t2packetpos << " len: " << len << " size: " << t2packetsize << std::endl;
		memcpy(t2packet+t2packetpos,buf,len);
		t2packetpos+=len;
	}
}



int main(int argc, char** argv) {
	inbuf = &cin;
	outbuf = &cout;
	if(argc!=3) 
	{
		
		fprintf(stderr, "Usage: %s [PID] [PLP-ID]\n", argv[0]);
		return 1;
	}
	plpId=atoi(argv[2]);
	size=200;
	char* packets= new char[size*188];
	active=false;
	t2packet =new char[0x5009];
	syncB = new char[1];
	syncB[0]=0x47;
	while(1) {
		inbuf->read(packets,size * 188);
		if(inbuf->fail()) break;
		unsigned char *p;
		//outbuf->write(packets,size*188);
		for(int i=0; i<size*188;i+=188) {
				int start=0;
				/*if((((packets[i+3])&0xC0))!=0x00) {
					std::cerr << "encrypted" << std::endl;	
					continue;
				}*/
				int pid=(unsigned char)(packets[i+1]);
				pid&=0x01F;
				pid<<=8;
				pid|=(unsigned char)(packets[i+2]);
				
				if(pid!=atoi(argv[1])) {
					continue;

				}
				if((((packets[i+3])&0x30)>>4)==0x03) {
					start++;
					//printf("%i<>",static_cast<unsigned char>(packet[4]));
					start+=static_cast<unsigned char>(packets[i+4]); //adaption + payload, offset addieren
					if(start> 183) {
						std::cerr << "wrong AF" << std::endl;	
						continue;
					}
				}
				else if(((((packets[i + 3])&0x30)>>4)==0x02)) {
					std::cerr<< "cont " << i << std::endl;
					continue; // nur adaption field
				}
				if((((packets[i+1])&0x40)>>4)==0x04) { //START INDICATOR
					processt2(&packets[i+4+start],184-start, true);
				} else {
					processt2(&packets[i+4+start],184-start, false);
				}
		}
	}
	return 0;
}
