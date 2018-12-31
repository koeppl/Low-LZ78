#ifndef CDSLIB_TRIE_HPP
#define CDSLIB_TRIE_HPP

#include <sdsl/int_vector.hpp>
#include <map>
#include <queue>


namespace cdslib {

    // Declaration of a node of the trie
    template<class value_type>
    struct trie_node {
        uint64_t id;
        std::map<value_type, trie_node> children;

        //! Constructor
        trie_node(){ }

        //! Constructor
        trie_node(uint64_t id, std::map<value_type, trie_node> children):
                id(id), children(children){};

        //! Constructor
        trie_node(uint64_t id): id(id) {
            std::map<value_type, trie_node> ch;
            children = ch;
        }

        //! Destructor
        ~trie_node() {}

        //! Copy constructor
        trie_node(const trie_node& v) = default;

        //! Move copy constructor
        trie_node(trie_node&& v) = default;

        //! Equality operator.
        bool operator == (const trie_node& v) const {
            return id == v.id;
        }

        //! Inequality operator.
        bool operator!=(const trie_node& v)const {
            return !(*this == v);
        }

        //! Assignment operator.
        trie_node& operator=(const trie_node& v) = default;
        //! Move assignment
        trie_node& operator=(trie_node&& v) = default;

        size_t sizebytes() {
            size_t s = sizeof(uint64_t) + (children.size()) * sizeof(value_type);
            for(auto it = children.begin(); it != children.end(); ++it) {
                s += (it->second).sizebytes();
            }
            return s;
        }

    };


    template<uint8_t t_width>
    class trie {
    private:
        static_assert(t_width <= 64, "trie: width of elements of the trie must be at most 64bits.");

    public:
        typedef typename sdsl::int_vector<>::size_type size_type;
        typedef typename sdsl::int_vector_trait<t_width>::value_type value_type;
        typedef trie_node<value_type> node_type;

    private:
        node_type t_root;
        size_type next_id; //for lz-trie also indicates the number of diff. phrases
        std::map<value_type, size_type>     alphabet_values;

        //only used to write the lz78 to disk
        sdsl::int_vector<>                  alphabet;
        uint64_t *buffer;
        uint64_t pos_buffer;

    public:
        const node_type &root = t_root;
        const size_type &n_nodes = next_id;

        trie() {
            std::map<value_type, node_type> children;
            t_root = node_type(0, children);
            next_id = 1;
        }

        void
        swap(trie &t) {
            if (this != &t) {
                std::swap(t_root, t.t_root);
                std::swap(next_id, t.next_id);

                alphabet.swap(t.alphabet);
                alphabet_values.swap(t.alphabet_values);
            }
        }

        //! Returns the number of nodes of the trie
        size_type
        size() {
            return next_id;
        }

        //Create the lz trie from a text (note that in this case value_type is a char)
        void
        generate_lz_trie(std::istream &in) {
            node_type *node = &t_root;
            value_type v;
            char c;
            while (in.get(c)) {
                v = (value_type) c;
                auto it = node->children.find(v);
                if (it == node->children.end()) {
                    insert(v, *node);
                    node = &t_root; //start again from the root
                }
                else
                    node = &(it->second);
            }
            std::cout << "  Number of diff phrases (of nodes): " << next_id << std::endl;
            std::cout << "  Memory in bytes used by the trie: " << t_root.sizebytes() << std::endl;
        }

        //! Represents the trie with parentheses, letters and ids
        void
        representTrie(sdsl::bit_vector &paren,
                      sdsl::int_vector<> &letters,
                      sdsl::int_vector<> &ids) {
            size_type pi = 0, pli = 0;
            sdsl::bit_vector pa(2 * next_id, 0);
            sdsl::int_vector<> le(next_id, 0, sizeof(value_type));
            sdsl::int_vector<> id(next_id, 0, sizeof(size_type));
            le[0] = 0; // dummy value
            traverseTrie(&t_root, pa, le, id, pi, pli);
            paren.swap(pa);
            letters.swap(le);
            ids.swap(id);
        }

        void
        createLZtoFile(std::istream& in, std::ostream& out, size_type sigma) {
            size_type len_file, bits_pos, bits_value, bits_len, further_pos;
            size_type count_sigma = 0;
            node_type *node = &t_root;
            alphabet = sdsl::int_vector<>(sigma, 0, t_width);
            pos_buffer = 0;
            bits_pos = 1; //sdsl::bits::hi(len_file) + 1;
            further_pos = 1;
            //bits_len = 1; //pos and len depend on where we are in the text
            bits_value = sdsl::bits::hi(sigma) + 1;
            buffer = cdslib::init_buffer();
            value_type v;
            char c;
            size_type text_pos = 0, len = 0, ini_pos = 0;
            while (in.get(c)) {
                v = (value_type) c;
                if (count_sigma < sigma) {
                    auto it = alphabet_values.find(v);
                    if (it == alphabet_values.end()) {
                        alphabet_values[v] = count_sigma + 1;
                        alphabet[count_sigma] = v;
                        ++count_sigma;
                    }
                }
                if (node->children.find(v) == node->children.end()) {
                    //write (node->id,len,v)
                    if(ini_pos > further_pos) {
                        bits_pos = sdsl::bits::hi(ini_pos) + 1;
                        further_pos = (1 << bits_pos) - 1;
                    }
                    saveValue(bits_pos, node->id, len, bits_value, v);
                    cdslib::check_buffer(buffer, pos_buffer, out);
                    insert_and_write(v, *node, ini_pos);
                    node = &t_root; //start again from the root
                    len = 0;
                    ini_pos = text_pos;
                }
                else {
                    node = &(node->children[v]);
                    ++ len;
                }
                ++ text_pos;
            }
            //check if the last phrase was wrote (should never happen if the text finished
            // with a special symbol)
            if (len > 0) {
                if(ini_pos > further_pos)
                    bits_pos = sdsl::bits::hi(ini_pos) + 1;
                saveValue(bits_pos, node->id, len, bits_value, sigma - 1); //write  not valid value
                std::cout << "error" << std::endl;
            }
            if (pos_buffer != 0)
                cdslib::SaveValue(out, buffer, (pos_buffer + 63) / 64);
            delete [] buffer;

            std::cout << "  Number of diff phrases (of nodes): " << next_id << std::endl;
            std::cout << "  Memory in bytes used by the trie: " << t_root.sizebytes() << std::endl;
        }

    private:

        void
        traverseTrie(node_type *node, sdsl::bit_vector &paren,
                      sdsl::int_vector<> &letters,
                      sdsl::int_vector<> &ids, size_type &pi, size_type &pli) {
            ++pi; //leave paren[pi] in 0
            ids[pli] = node->id;
            for (auto it = node->children.begin(); it != node->children.end(); ++it) {
                ++pli;
                letters[pli] = it->first;
                traverseTrie(&(it->second), paren, letters, ids, pi, pli);
            }
            paren[pi] = 1;
            ++pi;
        }

        //inserts when we know that does not exist
        void
        insert(value_type c, node_type &v) {
            alphabet_values[c] = true;
            node_type new_node(next_id/*, c*/);
            v.children.insert(std::pair<value_type, node_type>(c, new_node));
            ++next_id;
        }

        //inserts when we know that does not exist
        void
        insert_and_write(value_type c, node_type &v, size_type pos) {
            alphabet_values[c] = true;
            node_type new_node(pos/*, c*/);
            v.children.insert(std::pair<value_type, node_type>(c, new_node));
            ++next_id;
        }

        void
        saveValue(size_type bits_pos, size_type pos, size_type len,
                  size_type bits_value, size_type value) {
            cdslib::set_field(buffer, pos_buffer, pos_buffer + bits_pos, pos);
            pos_buffer += bits_pos;
            // store len using log(ini_pos - pos) + 1 bits
            cdslib::set_field(buffer, pos_buffer, pos_buffer + bits_pos, len);
            pos_buffer += bits_pos;
            // store value using log(sigma)
            cdslib::set_field(buffer, pos_buffer, pos_buffer + bits_value,
                                      alphabet_values[value]);
            pos_buffer += bits_value;
        }

    }; //end class trie


} // end namespace cdslib

#endif //CDSLIB_TRIE_HPP