#include "parser.h"

// Возможные функции поиска:

// Начало ключевой зоны:
unsigned int standartFindBeginKeyAreaPosition(const std::string& s, unsigned int begin_pos, const std::string& key_name, const ParserTree& tree)
{
    unsigned int whitespace_pos = key_name.find(' ');
    const std::string& key_word = key_name.substr(0, whitespace_pos - 1);
    unsigned int find1 = s.find(key_word + '>', begin_pos);
    unsigned int find2 = s.find(key_word + ' ', begin_pos);
    return find1 < find2 ? find1 : find2;
}

unsigned int rootItemFindBeginKeyAreaPosition(const std::string& s, unsigned int begin_pos, const std::string& key_name, const ParserTree& tree)
{
    return 0;
}


// Начало данных ключа:
unsigned int standartFindBeginDataPosition(const std::string& s, unsigned int begin_key_area_pos, const std::string& key_name, const ParserTree& tree)
{
    return s.find('>', begin_key_area_pos) + 1;
}

unsigned int rootItemFindBeginDataPosition(const std::string& s, unsigned int begin_key_area_pos, const std::string& key_name, const ParserTree& tree)
{
    return 0;
}


// Конец данных ключа:
// Необходимый прототип
unsigned int standartFindEndKeyAreaPosition(const std::string& s, unsigned int end_data_pos, const std::string& key_name, const ParserTree& tree);

unsigned int standartFindEndDataPosition(const std::string& s, unsigned int begin_data_pos, const std::string& key_name, const ParserTree& tree)
{
    int count_nested_same_name_keys = 0;
    unsigned int whitespace_pos = key_name.find(' ');
    const std::string end_key_str = key_name.substr(whitespace_pos + 1);
    unsigned int find_begin, find_end;
    unsigned int end_key_word;

    find_begin = standartFindBeginKeyAreaPosition(s, begin_data_pos, key_name, tree);
    find_end = s.find(end_key_str, begin_data_pos);

    /* Treat nested keys */
    while (find_begin < find_end)
    {
        count_nested_same_name_keys++;
        /* Find next pair start_key - finish_key */
        end_key_word = standartFindBeginDataPosition(s, find_begin, key_name, tree);
        find_begin = standartFindBeginKeyAreaPosition(s, end_key_word, key_name, tree);
        end_key_word = standartFindEndKeyAreaPosition(s, find_end, key_name, tree);
        find_end = s.find(end_key_str, end_key_word);
    }

    return find_end;
}

unsigned int rootItemFindEndDataPosition(const std::string& s, unsigned int begin_data_pos, const std::string& key_name, const ParserTree& tree)
{
    return s.size();
}


// Конец ключевой зоны:
unsigned int standartFindEndKeyAreaPosition(const std::string& s, unsigned int end_data_pos, const std::string& key_name, const ParserTree& tree)
{
    return s.find('>', end_data_pos) + 1;
}

unsigned int rootItemFindEndKeyAreaPosition(const std::string& s, unsigned int end_data_pos, const std::string& key_name, const ParserTree& tree)
{
    return s.size();
}

//=======================================================

// Поиск подходящих функций:
const KeyType::SearchPositionFunctions findSearchFunction(const std::string& key_name)
{
    if (key_name == "")
    {
        KeyType::SearchPositionFunctions result = {
            rootItemFindBeginKeyAreaPosition, rootItemFindBeginDataPosition,
            rootItemFindEndDataPosition, rootItemFindEndKeyAreaPosition
        };
        return result;
    }

    KeyType::SearchPositionFunctions result = {
        standartFindBeginKeyAreaPosition, standartFindBeginDataPosition,
        standartFindEndDataPosition, standartFindEndKeyAreaPosition
    };
    return result;
}
