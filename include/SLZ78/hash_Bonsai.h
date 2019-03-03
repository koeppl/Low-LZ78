//
// Wrap code of Poyias code from https://github.com/Poyias/mBonsai
// Manage the D array using a hash table and a sublayer array
//

#ifndef CDSLIB_HASH_B_H
#define CDSLIB_HASH_B_H


#include <tudocomp/util/heap_size.hpp>
#include <tudocomp/util/compact_hash/map/typedefs.hpp>
#include <tudocomp/util/serialization.hpp>
#include <sdsl/int_vector.hpp>
#include <separate/separate_chaining_table.hpp>

using namespace separate_chaining;
class SeparateChainingTable {
    public:
    typedef typename sdsl::int_vector<>::size_type  size_type;
    typedef separate_chaining_map<varwidth_bucket, plain_bucket<size_type>, xorshift_hash<size_type>, incremental_resize> hashtable_type;

    hashtable_type m_table;

    SeparateChainingTable() : m_table(2) {}

    SeparateChainingTable(size_type M) 
        : m_table(M)
    { }

    void swap(SeparateChainingTable& o) {
        std::swap(m_table, o.m_table);
    }

		//TODO
    void load(std::istream& is) {
        size_t elements;
        is >> elements;
        for(size_t i = 0; i < elements; ++i) {
            hashtable_type::key_type key;
            hashtable_type::value_type value;
            is >> key;
            is >> value;
            m_table[key] = value;
        }
    }

    size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="") const {
        const size_t elements = m_table.size();
        size_t counter = 0;
        out << elements;
        for(auto it = m_table.cbegin_nav(); it != m_table.cend_nav(); ++it) {
            out << it.key();
            out << it.value();
            ++counter;
        }
        DCHECK_EQ(counter, elements);
        sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
        const size_type written_bytes = sizeof(size_t) + elements* (sizeof(hashtable_type::key_type) + sizeof(hashtable_type::value_type));
        sdsl::structure_tree::add_size(child, written_bytes );
        return written_bytes;
    }

    size_type getValue(size_type pos) {
        return m_table[pos];
    }

    void setValue(size_type key, size_type value) {
        // std::cout << "keywidth " << ((size_t)m_table.key_bit_width()) << std::endl;
        if(tdc::bits_for(key) > m_table.key_bit_width()) {
            hashtable_type table(tdc::bits_for(key));

            DCHECK_LE(m_table.key_bit_width(), table.key_bit_width());
            DCHECK_LE(tdc::bits_for(key), table.key_bit_width());
            for(auto it = m_table.rbegin_nav(); it != m_table.rend_nav(); --it) {
                table[it.key()] = static_cast<size_t>(it.value());
                m_table.erase(it);
            }
            DCHECK(m_table.empty());
            m_table.swap(table);
        }
        m_table[key] = value;
        // std::cout << "m_table[" << key << " <- " << value  << ", keywidth = " << ((size_t)m_table.key_bit_width()) << std::endl;
    }
    size_type get_size() const {
        return m_table.size();
    }
    // bool is_full() {
    //     return m_table.size() == m_table.table_size();
    // }

};


class BonsaiTable {
    public:
    typedef typename sdsl::int_vector<>::size_type  size_type;
    typedef tdc::compact_hash::map::plain_elias_hashmap_t<size_type> hashtable_type;

    hashtable_type m_table;

    BonsaiTable() {}

    BonsaiTable(size_type M, size_type N) 
        : m_table(0, tdc::bits_for(N))
    { }

    BonsaiTable(size_type M) 
        : m_table(0, tdc::bits_for(M))
    { }

    void swap(BonsaiTable& o) {
			std::swap(m_table, o.m_table);
    }

		//TODO
    void load(std::istream& is) {
        m_table = tdc::serialize<hashtable_type>::read(is);
    }

		//TODO
    size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="") const {
        sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));

        const size_type written_bytes = tdc::heap_size<hashtable_type>::compute(m_table).size_in_bytes();
        tdc::serialize<hashtable_type>::write(out, m_table);
        sdsl::structure_tree::add_size(child, written_bytes);
        return written_bytes;
    }

    size_type getValue(size_type pos) {
        return m_table[pos];
    }

    void setValue(size_type pos, size_type d) {
        m_table[pos] = d;
    }
    size_type get_size() const {
        return m_table.size();
    }
    bool is_full() {
        return m_table.size() == m_table.table_size();
    }

};



namespace cdslib {

    class hash_Bonsai {
    public:
        typedef typename sdsl::int_vector<>::size_type  size_type;

    private:

        sdsl::int_vector<> D0;                  //array containing collision information
        SeparateChainingTable D1;                //sublayer displacement variables
        std::map<size_type, size_type> mapSl;   // special cases that do not fit in D0 adn D1
        size_type max_d;        // upper bound value (not included) than can be stored into D0

    public:

        // Empty constructor
        hash_Bonsai() {
        }

        //t_d_bits must be smaller than 7, but we are only using value 3 
        hash_Bonsai(size_type M, double factor, size_type t_d_bits) {
            t_d_bits = 3;
            max_d = (1 << t_d_bits) - 1;
            D0 = sdsl::int_vector<>(M, 0, t_d_bits);
            D1 = SeparateChainingTable(2);
        }

        void
        swap(hash_Bonsai& hd) {
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
                // if (D1.is_full()) {
                //     size_type D_size = D0.size();
                //     size_type d_value;
                //     BonsaiTable D2 = BonsaiTable(D_size, 2 * D1.get_size());
                //     for (size_type i = 0; i < D_size; ++i) {
                //         d_value  = getValue(i);
                //         if (d_value >= max_d and d_value <= 134) {
                //             D2.setValue(i, d_value - max_d);
                //         }
                //     }
                //     D1.swap(D2);
                // }
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




#endif //CDSLIB_HASH_B_H