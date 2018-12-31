//
// Wrap code of Poyias code from https://github.com/Poyias/mBonsai
// Manage the D array using a hash table and a sublayer array
//

#ifndef CDSLIB_HASH_D_H
#define CDSLIB_HASH_D_H


#include "tudocomp/util/compact_sparse_hash.hpp"
#include <tudocomp/util/serialization.hpp>
#include <sdsl/int_vector.hpp>


class ShohinBonsai {
    public:
    typedef typename sdsl::int_vector<>::size_type  size_type;
    typedef tdc::compact_sparse_hashmap::compact_sparse_hashmap_t<size_type> hashtable_type;

    hashtable_type table;

    ShohinBonsai() {}

    ShohinBonsai(size_type M, size_type N) 
        : table(0, tdc::bits_for(N))
    { }

    ShohinBonsai(size_type M) 
        : table(0, tdc::bits_for(M))
    { }
    void swap(ShohinBonsai& o) {
        std::swap(table, o.table);
    }
//TODO
    void load(std::istream& is) {
        table = tdc::serialize<hashtable_type>::read(is);
    }
//TODO
    size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="") const {
        sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));

        tdc::serialize<hashtable_type>::write(out, table);
        size_type written_bytes = 0;
        sdsl::structure_tree::add_size(child, written_bytes);
        return written_bytes;
    }

    size_type getValue(size_type pos) {
        return table[pos];
    }

    void setValue(size_type pos, size_type d) {
        table[pos] = d;
    }
    size_type get_size() const {
        return table.size();
    }
    bool is_full() {
        return table.size() == table.table_size();
    }

};


namespace cdslib {

    class hash_D {
    public:
        typedef typename sdsl::int_vector<>::size_type  size_type;

    private:

        sdsl::int_vector<> D0;                  //array containing collision information
        ShohinBonsai D1;                //sublayer displacement variables
        std::map<size_type, size_type> mapSl;   // special cases that do not fit in D0 adn D1
        size_type max_d;        // upper bound value (not included) than can be stored into D0

    public:

        // Empty constructor
        hash_D() {
        }

        //t_d_bits must be smaller than 7, but we are only using value 3 
        hash_D(size_type M, double factor, size_type t_d_bits) {
            t_d_bits = 3;
            max_d = (1 << t_d_bits) - 1;
            D0 = sdsl::int_vector<>(M, 0, t_d_bits);
            D1 = ShohinBonsai(M);
        }

        void
        swap(hash_D& hd) {
            if (this != &hd) {
                D0.swap(hd.D0);
                std::swap(max_d, hd.max_d);
                D1.swap(hd.D1);
                mapSl.swap(hd.mapSl);
            }
        }


        size_type
        serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="") const {
            sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0, e_bytes = 0;
            written_bytes += write_member(max_d, out, child, "maximum value in D");
            written_bytes += D0.serialize(out, child, "D array");
            e_bytes += D1.serialize(out, child, "D1 array");
            { //save map
                size_type length_map = mapSl.size(), cont = 0;
                //std::cout << "number of special cases: " << length_map << std::endl;
                sdsl::int_vector<> elements(length_map, 0, 8 * sizeof(size_type));
                for(auto it = mapSl.begin(); it != mapSl.end(); it++) {
                    elements[cont] = it->first;
                    cont ++;
                }
                e_bytes += elements.serialize(out, child, "map keys");
                cont = 0;
                for(auto it = mapSl.begin(); it != mapSl.end(); it++) {
                    elements[cont] = it->second;
                    cont ++;
                }
                e_bytes += elements.serialize(out, child, "map values");
            }
            written_bytes += e_bytes;
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        void
        load(std::istream& in) {
            sdsl::read_member(max_d, in);
            D0.load(in);
            D1.load(in);
            sdsl::int_vector<> keys;
            sdsl::int_vector<> values;
            keys.load(in);
            values.load(in);
            for(size_type i = 0; i < keys.size(); ++i)
                mapSl[keys[i]] = values[i];
        }


        size_type
        getValue(size_type pos) {
            if (D0[pos] < max_d)
                return D0[pos];
            else {
                auto it = mapSl.find(pos);
                if (it != mapSl.end())  //d value was over 135
                    return it->second;
                else //it is in D1
                    return D1.getValue(pos) + max_d;
            }
        }

        void
        setValue(size_type pos, size_type d) {
            if (d < max_d)
                D0[pos] = d;
            else {
                if (D1.is_full()) {
                    size_type D_size = D0.size();
                    size_type d_value;
                    ShohinBonsai D2 = ShohinBonsai(D_size, 2 * D1.get_size());
                    for (size_type i = 0; i < D_size; ++i) {
                        d_value  = getValue(i);
                        if (d_value >= max_d and d_value <= 134) {
                            D2.setValue(i, d_value - max_d);
                        }
                    }
                    D1.swap(D2);
                }
                D0[pos] = max_d;
                if (d > 134) //from the original implementation (7 bits for this level)
                    mapSl[pos] = d;
                else
                    D1.setValue(pos, d - max_d);
            }
        }

    private:


    };


}




#endif //CDSLIB_HASH_D_H
