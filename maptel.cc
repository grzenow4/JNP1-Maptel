#include <cassert>
#include <cstring>
#include <set>
#include <sstream>
#include <unordered_map>
#include "maptel.h"

// Typ reprezentujący słownik telefonów.
using dictionary_t = std::unordered_map <std::string, std::string>;

// Typ reprezentujący id słowników telefonów.
using dictionary_id_t = unsigned long;

// Stała sprawdzająca, czy wypisywać informacje diagnostyczne
// i sprawdzać, czy wystąpiły błędy.
#ifdef NDEBUG
const bool debug = false;
#else
const bool debug = true;
#endif

namespace {
    // Funkcja przechowująca mapę słowników telefonów.
    std::unordered_map <dictionary_id_t, dictionary_t> &dictionaries_wrapper() {
        static std::unordered_map <dictionary_id_t, dictionary_t> dictionaries;
        return dictionaries;
    }

    // Funkcja sprawdzająca, czy telefon jest poprawny.
    bool is_tel_valid(char const *tel) {
        if (tel == NULL || strlen(tel) > ::jnp1::TEL_NUM_MAX_LEN) {
            return false;
        }

        size_t i;
        for (i = 0; i < ::jnp1::TEL_NUM_MAX_LEN; i++) {
            if (tel[i] == '\0') {
                break;
            }
            else if (!std::isdigit(tel[i])) {
                return false;
            }
        }

        if (i == 0) {
            return false;
        }

        return true;
    }
}

unsigned long ::jnp1::maptel_create(void) {
    static dictionary_id_t id_g = 0;

    if (debug) {
        std::cerr << "maptel: maptel_create()\n";
    }

    auto &dictionaries = ::dictionaries_wrapper();
    dictionaries[id_g];

    if (debug) {
        std::cerr << "maptel: maptel_create: new map id = " << id_g << "\n";
    }

    id_g++;

    return id_g - 1;
}

void ::jnp1::maptel_delete(dictionary_id_t id) {
    if (debug) {
        std::cerr << "maptel: maptel_delete(" << id << ")\n";
    }

    auto &dictionaries = ::dictionaries_wrapper();
    auto it = dictionaries.find(id);

    if (debug) {
        assert(it != dictionaries.end());
    }

    it->second.clear();
    dictionaries.erase(it->first);

    if (debug) {
        std::cerr << "maptel: maptel_delete: map " << id << " deleted\n";
    }
}

void ::jnp1::maptel_insert(dictionary_id_t id, char const *tel_src, char const *tel_dst) {
    if (debug) {
        std::cerr << "maptel: maptel_insert(";
        std::cerr << id << ", " << tel_src << ", " << tel_dst << ")\n";
        assert(is_tel_valid(tel_src));
        assert(is_tel_valid(tel_dst));
    }

    auto &dictionaries = ::dictionaries_wrapper();
    auto it = dictionaries.find(id);

    if (debug) {
        assert(it != dictionaries.end());
    }

    std::stringstream src;
    std::stringstream dst;
    src << tel_src;
    dst << tel_dst;

    it->second[src.str()] = dst.str();

    if (debug) {
        std::cerr << "maptel: maptel_insert: inserted\n";
    }
}

void ::jnp1::maptel_erase(dictionary_id_t id, char const *tel_src) {
    if (debug) {
        std::cerr << "maptel: maptel_erase(" << id << ", " << tel_src << ")\n";
        assert(is_tel_valid(tel_src));
    }

    std::stringstream src;
    src << tel_src;

    auto &dictionaries = ::dictionaries_wrapper();
    auto it = dictionaries.find(id);

    if (debug) {
        assert(it != dictionaries.end());
    }

    auto inner_it = it->second.find(src.str());
    auto inner_end = it->second.end();

    if (inner_it != inner_end) {
        it->second.erase(inner_it);
    }

    if (debug) {
        std::cerr << "maptel: maptel_erase: ";
        if (inner_it == inner_end) {
            std::cerr << "nothing to erase\n";
        } else {
            std::cerr << "erased\n";
        }
    }
}

void ::jnp1::maptel_transform(dictionary_id_t id, char const *tel_src, char *tel_dst, std::size_t len) {
    if (debug) {
        std::cerr << "maptel: maptel_transform(" << id << ", ";
        std::cerr << tel_src << ", " << &tel_dst << ", " << len << ")\n";
        assert(is_tel_valid(tel_src));
        assert(tel_dst != NULL);
    }

    std::set<std::string> visited_numbers = std::set<std::string>();
    std::stringstream dst;
    dst << tel_src;
    auto &dictionaries = ::dictionaries_wrapper();
    auto it = dictionaries.find(id);

    if (debug) {
        assert(it != dictionaries.end());
    }

    auto inner_it = it->second.find(dst.str());
    auto inner_end = it->second.end();
    std::string src = dst.str();
    visited_numbers.insert(src);

    while (inner_it != inner_end) {
        src = inner_it->second;
        inner_it = it->second.find(src);
        if (visited_numbers.count(src) == 1) {
            if (debug) {
                std::cerr << "maptel: maptel_transform: cycle detected\n";
            }
            src = dst.str();
            break;
        }
        visited_numbers.insert(src);
    }

    if (debug) {
        assert(src.size() < len);
    }

    std::copy(src.begin(), src.end(), tel_dst);
    tel_dst[src.size()] = 0;

    if (debug) {
        std::cerr << "maptel: maptel_transform: ";
        std::cerr << tel_src << " -> " << tel_dst << "\n";
    }
}
