//
// Created by rcanovas on 14/11/16.
//

#ifndef CDSLIB_MAP_D_H
#define CDSLIB_MAP_D_H

#include "./../tools.h"
#include <sdsl/coder_elias_gamma.hpp>

namespace cdslib {

    class map_D {
    public:
        typedef typename sdsl::int_vector<>::size_type  size_type;

    private:
        sdsl::int_vector<> D;               //array containing collision information
        std::map<size_type, size_type> E;   //special case collisions
        size_type max_d;                    // maximum value stored in D

    public:

        // Empty constructor
        map_D() { }

        // (t_d_bits = 3 bits original code)
        map_D(size_type M, double factor, size_type t_d_bits = 0) {
            size_type bits_D = t_d_bits;
             if (bits_D == 0) {
                 max_d = (size_type) (1.0 / factor);
                 bits_D = sdsl::bits::hi(max_d) + 1;
             }
             else
                 max_d = (1 << bits_D) - 1;
            std::cout << "max_d: " << max_d << "  bits D: " << bits_D << std::endl;
            D = sdsl::int_vector<>(M, 0, bits_D);
        }

        void
        swap(map_D& md) {
            if (this != &md) {
                D.swap(md.D);
                std::swap(max_d, md.max_d);
                //std::cout << "E size :  " << E.size();
                E.swap(md.E);
                //std::cout << "  new:  " << E.size();
            }
        }

				size_type 
				size_in_bytes() {
					size_type mem = sdsl::size_in_bytes(D);
					size_type length_map = E.size();
					mem += length_map * (sizeof(size_type) * 2 + 24);
					return mem;
				}


        size_type
        serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="") const {
            sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0, e_bytes = 0;
            written_bytes += write_member(max_d, out, child, "maximum value in D");
            // written_bytes += D.serialize(out, child, "D array");
			{
				sdsl::int_vector<> prepD = D;
				for(size_t it = 0; it < prepD.size(); ++it) {
					++prepD[it];
				}
				sdsl::int_vector<> serD;
				sdsl::coder::elias_gamma::encode(prepD, serD);
				const size_t written_b = serD.serialize(out, child, "D array");
				written_bytes += written_b;
				std::cout << "size of fixed bit width D array: " << sdsl::size_in_bytes(D) << std::endl;
				std::cout << "size of gamma compressed fixed bit width D array: " << sdsl::size_in_bytes(serD) << std::endl;
				std::cout << "bytes for serialization of gamma compressed D array: " << written_bytes << std::endl;
				size_t zeros = 0;
				for(size_t it = 0; it < D.size(); ++it) {
					if(D[it] == 0) ++zeros;
				}
				std::cout << "zeros in D: " << zeros << " faction: " << (zeros*100/D.size()) << std::endl;
			}

            {//Save E
                size_type length_E = E.size(), cont = 0;
                std::cout << "number of special cases: " << length_E << std::endl;
                sdsl::int_vector<> elements(length_E, 0, 8*sizeof(size_type));
                for(auto it = E.begin(); it != E.end(); it++) {
                    elements[cont] = it->first;
                    cont ++;
                }
                e_bytes = elements.serialize(out, child, "map keys");
                cont = 0;
                for(auto it = E.begin(); it != E.end(); it++) {
                    elements[cont] = it->second;
                    cont ++;
                }
                e_bytes += elements.serialize(out, child, "map values");
            }
            std::cout << "Space used by special cases: " << e_bytes << " bytes" << std::endl;
            written_bytes += e_bytes;
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        void
        load(std::istream& in) {
            sdsl::read_member(max_d, in);
            //D.load(in);
			{
				sdsl::int_vector<> serD;
				serD.load(in);
				sdsl::coder::elias_gamma::decode(serD, D);
				for(size_t it = 0; it < D.size(); ++it) {
					--D[it];
				}
			}
            sdsl::int_vector<> keys;
            sdsl::int_vector<> values;
            keys.load(in);
            values.load(in);
            for(size_type i = 0; i < keys.size(); ++i)
                E[keys[i]] = values[i];
        }


        size_type
        getValue(size_type pos) {
            if (D[pos] == max_d) {
                auto it = E.find(pos); // assume that always find it
                return it->second;
            }
            else
                return D[pos];
        }

        void
        setValue(size_type pos, size_type d) {
            if (d >= max_d) {
                D[pos] = max_d;
                E[pos] = d;
            }
            else
                D[pos] = d;
        }

    };


}

#endif //CDSLIB_MAP_D_H
