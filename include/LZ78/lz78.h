//
// Created by rcanovas on 21/03/17.
//

#ifndef CDSLIB_LZ78_H
#define CDSLIB_LZ78_H

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <map>
#include <stack>
#include "trie.h"

namespace cdslib {

    template<uint8_t t_width = 8>
    class lz78 {

    public:
        typedef typename sdsl::int_vector<>::size_type                  size_type;
        typedef typename sdsl::int_vector_trait<t_width>::value_type    value_type;

    private:
        size_type sigma;

        uint64_t *buffer;
        uint64_t pos_buffer;

    public:

        // Empty constructor
        lz78() {

        }

        //! Compress file_name into out_file
        //  s : size of the alphabet
        void
        compress_file(std::string file_name, std::string out_file, size_type s, double factor,
                      size_type bits_d, double _gf = 2.0) {
            sigma = s + 1; //add an special case
            std::ofstream f_out(out_file);
            std::ifstream f_in(file_name, std::ios::in | std::ios::binary);
            if(!f_in) {
                std::cerr << "Failed to open file " << file_name;
                exit(1);
            }
            process_text(f_in, f_out);
        }


        //! Decompress the stored text into out_file. Note that first the data structure must be loaded
        void
        decompress_file(std::istream& in, std::string out_file) {
					//not implemented
        }

        //! Loads the data structure
        void
        load(std::istream& in) {

        }

    private:


        void
        process_text(std::istream& in, std::ostream& out) {
            trie<t_width> T = trie<t_width>();
            T.createLZtoFile(in, out, sigma);
        }



    };

}

#endif //CDSLIB_LZ78_H
