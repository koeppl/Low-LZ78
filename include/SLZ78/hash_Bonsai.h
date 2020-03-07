//
// Wrap code of Poyias code from https://github.com/Poyias/mBonsai
// Manage the D array using a hash table and a sublayer array
//

#ifndef CDSLIB_HASH_B_H
#define CDSLIB_HASH_B_H


#include "./hash_D_level.h"
#include "defs.h"

#include <tudocomp/util/heap_size.hpp>
#include <tudocomp/util/compact_hash/map/typedefs.hpp>
#include <tudocomp/util/serialization.hpp>
#include <sdsl/int_vector.hpp>
#include <separate/separate_chaining_table.hpp>
#include <separate/group_chaining.hpp>

using namespace separate_chaining;

class SeparateChainingTable {
    public:
    typedef typename sdsl::int_vector<>::size_type  size_type;
#if defined(BONSAI_HASH_TABLE) && BONSAI_HASH_TABLE == 1
    typedef separate_chaining_map<varwidth_bucket<>, plain_bucket<size_type>, xorshift_hash<size_type>, incremental_resize> hashtable_type;
#else
	typedef group::group_chaining_table<> hashtable_type;
#endif

    hashtable_type m_table;

    SeparateChainingTable() {} //dummy

    SeparateChainingTable(uint_fast8_t key_width, uint_fast8_t value_width) 
        : m_table(key_width, value_width)
    { 
	}

    void swap(SeparateChainingTable& o) {
        std::swap(m_table, o.m_table);
    }
	std::string name() const { return std::string(typeid(*this).name()) + std::string(" ") + std::string(typeid(m_table).name()); }

		//TODO
    void load(std::istream& is) {
        // return load_naive(is);
        m_table.deserialize(is);
    }
    void load_naive(std::istream& is) {
        size_t elements;
        is.read(reinterpret_cast<char*>(&elements), sizeof(size_t));
        for(size_t i = 0; i < elements; ++i) {
            hashtable_type::key_type key;
            hashtable_type::value_type value;
            is.read(reinterpret_cast<char*>(&key), sizeof(hashtable_type::key_type));
            is.read(reinterpret_cast<char*>(&value), sizeof(hashtable_type::value_type));
            m_table[key] = value;
        }
    }
    // size_type serialize_naive(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="") const {
    //     const size_t elements = m_table.size();
    //     size_t counter = 0;
    //     out.write(reinterpret_cast<const char*>(&elements), sizeof(size_t));
    //
    //     for(auto it = m_table.cbegin_nav(); it != m_table.cend_nav(); ++it) {
    //         hashtable_type::key_type key = it.key();
    //         hashtable_type::value_type value = it.value();
    //         out.write(reinterpret_cast<const char*>(&key), sizeof(hashtable_type::key_type));
    //         out.write(reinterpret_cast<const char*>(&value), sizeof(hashtable_type::value_type));
    //         ++counter;
    //     }
    //     DCHECK_EQ(counter, elements);
    //     const size_type written_bytes = sizeof(size_t) + elements* (sizeof(hashtable_type::key_type) + sizeof(hashtable_type::value_type));
    //
    //     sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
    //     sdsl::structure_tree::add_size(child, written_bytes );
    //     return written_bytes;
    // }

    size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="") const {
        // return serialize_naive(out, v, name);
        const auto streamposition = out.tellp();
        m_table.serialize(out);
        const size_type written_bytes = out.tellp() - streamposition;

        sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
        sdsl::structure_tree::add_size(child, written_bytes );
        return written_bytes;
    }

    size_type getValue(size_type pos) {
        DCHECK_LE(tdc::bits_for(pos), m_table.key_width()); //such a value must not have been hashed!
        return m_table[pos];
    }

    void setValue(size_type key, size_type value) {
        // std::cout << "keywidth " << ((size_t)m_table.key_width()) << std::endl;
		DCHECK_LE(tdc::bits_for(key), m_table.key_width());
#if(0)
        if(tdc::bits_for(key) > m_table.key_width()) {
            hashtable_type table(tdc::bits_for(key));

            DCHECK_LE(m_table.key_width(), table.key_width());
            DCHECK_LE(tdc::bits_for(key), table.key_width());
            for(auto it = m_table.rbegin_nav(); it != m_table.rend_nav(); --it) {
                table[it.key()] = static_cast<size_t>(it.value());
                m_table.erase(it);
            }
            DCHECK(m_table.empty());
            m_table.swap(table);
        }
#endif//0
        m_table[key] = value;
        // std::cout << "m_table[" << key << " <- " << value  << ", keywidth = " << ((size_t)m_table.key_width()) << std::endl;
    }
    // size_type get_size() const {
    //     return m_table.size();
    // }
    // bool is_full() {
    //     return m_table.size() == m_table.table_size();
    // }

};


class BonsaiTable {
    public:
    typedef typename sdsl::int_vector<>::size_type  size_type;
    // typedef tdc::compact_hash::map::plain_elias_hashmap_t<size_type> hashtable_type; // eliasP
	typedef tdc::compact_hash::map::sparse_cv_hashmap_t<tdc::dynamic_t> hashtable_type; // clearyS 

    hashtable_type m_table;

	std::string name() const { return std::string(typeid(*this).name()) + std::string(" ") + std::string(typeid(m_table).name()); }

    BonsaiTable() {} //!dummy

    BonsaiTable(uint_fast8_t key_width, uint_fast8_t value_width) 
        : m_table(0, key_width, value_width)
    { 
	}

    // BonsaiTable(size_type M) 
    //     : m_table(0, tdc::bits_for(M))
    // { }
    //
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

    void setValue(size_type key, size_type d) {
		DCHECK_LE(tdc::bits_for(key), m_table.key_width());
#if(0)
        if(tdc::bits_for(key) > m_table.key_width()) {
			m_table.grow_key_width(tdc::bits_for(key));
		}
#endif//0
		m_table[key] = d;
    }
    // size_type get_size() const {
    //     return m_table.size();
    // }
    bool is_full() {
        return m_table.size() == m_table.table_size();
    }

};



namespace cdslib {
    class hash_Bonsai {
    public:
        typedef typename sdsl::int_vector<>::size_type  size_type;
#if defined(BONSAI_HASH_TABLE) && BONSAI_HASH_TABLE == 0
		typedef BonsaiTable hashtable_type;
#else
		typedef SeparateChainingTable hashtable_type;
#endif

    private:

        sdsl::int_vector<> D0;                  //array containing collision information
#ifndef NDEBUG
        cdslib::hash_D_level D1;                //sublayer displacement variables
#endif
        hashtable_type D4;
        std::map<size_type, size_type> mapSl;   // special cases that do not fit in D0 adn D1
        size_type max_d;        // upper bound value (not included) than can be stored into D0

    public:

        // Empty constructor
        hash_Bonsai() { }
		

        //t_d_bits = m_key_bits must be smaller than 7, but we are only using value 3 
		static constexpr size_type m_value_width = 3;
        hash_Bonsai(size_type M, [[maybe_unused]] double factor, [[maybe_unused]] size_type t_d_bits) 
			: D4(tdc::bits_for(M), m_value_width)
		{
            max_d = (1 << m_value_width) - 1;
            D0 = sdsl::int_vector<>(M, 0, m_value_width);
#ifndef NDEBUG
            D1 = hash_D_level(M);
#endif
            // D4 = hashtable_type(M, m_value_width); //TODO: parameter = maximal number of bits a key can have. Is this m_value_width??
			std::cout << "[Bonsai]: create table with key_width = " << ((size_t)tdc::bits_for(M)) << " and value_width = " << ((size_t)m_value_width) << std::endl;
			std::cout << "[Bonsai]: table = " << D4.name() << std::endl;
        }

        void
        swap(hash_Bonsai& hd) {
			std::cout << "[Bonsai]: swapping" << std::endl;
            if (this != &hd) {
                D0.swap(hd.D0);
                std::swap(max_d, hd.max_d);
#ifndef NDEBUG
                D1.swap(hd.D1);
#endif
                D4.swap(hd.D4);
                mapSl.swap(hd.mapSl);
            }
        }

				size_type size_in_bytes() {
					size_type mem = sdsl::size_in_bytes(D0);
					{
						std::ofstream f_tmp("temp_bonsai_ram_data");
						size_type d4 = D4.serialize(f_tmp);
						mem += d4;
						remove("temp_bonsai_ram_data");
					}
					size_type length_map = mapSl.size();
					mem += length_map * (sizeof(size_type) * 2 + 24); //lenght_map (key-values + 3 pointers)
					return mem;
				}

        size_type
        serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="") const {
			std::cout << "[Bonsai]: serializing" << std::endl;
            sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0, e_bytes = 0;
            written_bytes += write_member(max_d, out, child, "maximum value in D");
            written_bytes += D0.serialize(out, child, "D array");
#ifndef NDEBUG
            e_bytes += D1.serialize(out, child, "D1 array");
#endif
            size_type d4_size = D4.serialize(out, child, "D4 array");
						std::cout << "D4 size: " << d4_size << std::endl;
            e_bytes += d4_size;
						{ //save map
							size_type map_size = 0;
							size_type length_map = mapSl.size(), cont = 0;
							//std::cout << "number of special cases: " << length_map << std::endl;
							sdsl::int_vector<> elements(length_map, 0, 8 * sizeof(size_type));
							for(auto it = mapSl.begin(); it != mapSl.end(); it++) {
								elements[cont] = it->first;
								cont ++;
							}
							map_size += elements.serialize(out, child, "map keys");
							cont = 0;
							for(auto it = mapSl.begin(); it != mapSl.end(); it++) {
								elements[cont] = it->second;
								cont ++;
							}
							map_size += elements.serialize(out, child, "map values");
							//extra space used by the map (assuming that it uses a redblack tree ds--> 2 pointers (8bytes pp) + 1 pointer to the parent)
							//3*8*number of elements in the map
							map_size += 24 *  mapSl.size();
							e_bytes += map_size;
						}
            written_bytes += e_bytes;
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        void
        load(std::istream& in) {
			std::cout << "[Bonsai]: deserializing" << std::endl;
			std::cout << "[Bonsai]: table = " << typeid(D4).name() << std::endl;
            sdsl::read_member(max_d, in);
            D0.load(in);
#ifndef NDEBUG
            D1.load(in);
#endif
            D4.load(in);
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
                else {//it is in D1
#ifndef NDEBUG
                    DCHECK_EQ(D1.getValue(pos), D4.getValue(pos));
#endif
                    return D4.getValue(pos) + max_d;
                    }
            }
        }

        void
        setValue(size_type pos, size_type d) {
            if (d < max_d)
                D0[pos] = d;
            else {
#ifndef NDEBUG
                if (D1.is_full()) {
                    size_type D_size = D0.size();
                    size_type d_value;
                    cdslib::hash_D_level D2 = hash_D_level(D_size, 2 * D1.get_size());
                    for (size_type i = 0; i < D_size; ++i) {
                        d_value  = getValue(i);
                        if (d_value >= max_d and d_value <= 134) {
                            D2.setValue(i, d_value - max_d);
                        }
                    }
                    D1.swap(D2);
                }
#endif
                D0[pos] = max_d;
                if (d > 134) //from the original implementation (7 bits for this level)
                    mapSl[pos] = d;
                else {
                    D4.setValue(pos, d - max_d);
#ifndef NDEBUG 
                    D1.setValue(pos, d - max_d);
					DCHECK_EQ(D1.getValue(pos), D4.getValue(pos));
#endif
                }
            }
        }

    };





}//ns




#endif //CDSLIB_HASH_B_H
