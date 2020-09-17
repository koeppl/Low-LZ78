/* cdslib - compressed data structures library
    Copyright (C)2016 Rodrigo Canovas

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see http://www.gnu.org/licenses/ .
*/

#include <iostream>
#include <unistd.h>
#include "./../include/SLZ78/hlz78.h"
#include "./../include/SLZ78/mhlz78.h"
#include "./../include/SLZ78/ghlz78.h"
#include "./../include/SLZ78/ghlz78S.h"
#include "./../include/SLZ78/ghlz78S2.h"
#include "./../include/SLZ78/bhlz78.h"
#include "./../include/LZ78/lz78.h"


using namespace std;

template<class idx_type>
void
compress_text(std::string file, std::string out_file, double factor, size_t sigma, size_t d_bits, double gf) {
    auto idx = idx_type();
    idx.compress_file(file, out_file, sigma, factor, d_bits, gf);
   /*
    std::cout << "Size Index: " << sdsl::size_in_bytes(idx) << " bytes" << std::endl;
   */
}

int main(int argc, char* argv[]) {

    if(argc < 2) {
        cout << "Usage: " << argv[0] << " file_name <opt>" << endl;
        cout << "opt: " << endl;
        cout << "-o output_name:  String containing the name of the output file. Default file_name.hlz78" << endl;
        cout << "-s sigma: Size of the alphabet of the input text. Default s = 256" << endl;
        cout << "-f factor: Integer value indicating the overload factor used for the used hash table. Default factor = 5 (meaning 5%). The overload factor f determines the maximum load factor 1/f of the used hash table." << endl;
        cout << "-d D_bits: Number of bits used to store each collision value. Default d=0 and compute it internally" << endl;
				cout << "-g grow: Grow factor used for the hash table. Default g=2.0" <<endl;
        cout << "-w Index_type. Default = 0" << endl;
        cout << " # | Index_type" << endl;
        cout << "---+--------------------" << endl;
        cout << " 0 | HLZ78 using a map for the displacements" << endl;
        cout << " 1 | HLZ78 using a hash table and a sublayer for displacements" << endl;
        cout << " 2 | MHLZ78 using maps for the displacements" << endl;
        cout << " 3 | MHLZ78 using a hash table and a sublayer for displacements" << endl;
        cout << " 4 | GHLZ78 using maps for the displacements" << endl;
        cout << " 5 | GHLZ78 using a hash table and a sublayer for displacements" << endl;
        cout << " 6 | LZ78 classic construction" << endl;
        cout << " 7 | BHZ78 Brute force growing HLZ78 construction" << endl;
				cout << " 8 | GHLZ78(S)2 using a hash table and a sublayer for displacements and a Sampled node array when growing" << endl;
				cout << " 9 | GHLZ78 using a hash table and a sublayer for displacements" << endl;
				// cout << " 10 | HLZ78 using a hash table and a sublayer for displacements with predefined space" << endl;
				return 0;
    }

    string file = argv[1];
    string out_file = file;
    uint8_t w = 0;
    double factor = 0.05, grow_factor = 2.0;
    size_t sigma = 256;
    size_t d_bits = 0;

    int c;
    while((c = getopt (argc, argv, "o:w:f:g:s:d:")) != -1){
        switch (c) {
            case 'o': out_file = optarg;  break;
            case 'w': w = atoi(optarg); break;
            case 's': sigma = atoi(optarg); break;
            case 'f': factor = 1.0 * atoi(optarg) / 100.0; break;
            case 'g': grow_factor = atof(optarg); break;
            case 'd': d_bits = atoi(optarg); break;
            case '?': if(optopt == 'o' || optopt == 'w')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else
                    fprintf(stderr,"Unknown option character `\\x%x'.\n",	optopt);
                return 1;
            default:  abort ();
        }
    }

    //create index
    switch (w) {
        case 0:
			if(file == out_file)
            out_file += ".hlz78";
            compress_text<cdslib::hlz78<> >(file, out_file, factor, sigma, d_bits, grow_factor);
            break;
        case 1:  //using Bonsai hash
			if(file == out_file)
            out_file += ".hlz78_hash";
            compress_text<cdslib::hlz78<8, cdslib::hash_Bonsai> >(file, out_file, factor, sigma, d_bits, grow_factor);
            break;
        case 2:
			if(file == out_file)
            out_file += ".mhlz78";
            compress_text<cdslib::mhlz78<> >(file, out_file, factor, sigma, d_bits, grow_factor);
            break;
        case 3:
			if(file == out_file)
            out_file += ".mhlz78_hash";
            compress_text<cdslib::mhlz78<8, cdslib::hash_Bonsai> >(file, out_file, factor, sigma, d_bits, grow_factor);
            break;
        case 4:
			if(file == out_file)
            out_file += ".ghlz78";
            compress_text<cdslib::ghlz78<> >(file, out_file, factor, sigma, d_bits, grow_factor);
            break;
        case 5:
			if(file == out_file)
            out_file += ".ghlz78_hash";
            compress_text<cdslib::ghlz78<8, cdslib::hash_Bonsai> >(file, out_file, factor, sigma, d_bits, grow_factor);
            break;
        case 6:
			if(file == out_file)
            out_file += ".lz78";
            compress_text<cdslib::lz78<8>>(file, out_file, factor, sigma, d_bits, grow_factor);
            break;
        case 7:
			if(file == out_file)
			out_file += ".bhlz78";
			compress_text<cdslib::bhlz78<8, cdslib::hash_Bonsai>>(file, out_file, factor, sigma, d_bits, grow_factor);
			break;

		case 8:
			if(file == out_file)
			out_file += ".ghlz78S2_hash";
			compress_text<cdslib::ghlz78S2<8, cdslib::hash_Bonsai> >(file, out_file, factor, sigma, d_bits, grow_factor);
			break;
		case 9:
			if(file == out_file)
			out_file += ".ghlz78S2";
			compress_text<cdslib::ghlz78S2<8> >(file, out_file, factor, sigma, d_bits, grow_factor);
			break;
        // case 10:  //using Bonsai hash
		// 	if(file == out_file)
        //     out_file += ".hlz78_hash";
        //     compress_text<cdslib::hlz78<8, cdslib::hash_Bonsai> >(file, out_file, factor, sigma, d_bits, grow_factor);
        //     break;
		default:
			cout << "index_type must be a value in [0,8]" << endl;
			break;
    }

    return 0;
}
