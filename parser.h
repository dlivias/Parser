#ifndef PARSER_H
#define PARSER_H

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <stack>
#include <algorithm>
#include <iterator>

const std::string Tag_list_filename = "C:\\Users\\Admin\\Desktop\\parser_test\\tag list.txt";

class KeyType {
public:
    std::string name;
    bool operator<(KeyType key) const { return name < key.name; }
    explicit KeyType(const std::string& key_name) : name(key_name) {}
};

class KeyPositionType {
public:
    KeyType key;
    unsigned int begin_key_area_pos, end_key_area_pos;  // [...)
    unsigned int begin_data_pos, end_data_pos;          // [...)

    KeyPositionType(const KeyType& k, unsigned int begin_key_area_position, unsigned int end_key_area_position,
                        unsigned int begin_data_position, unsigned int end_data_position)
        : key(k), begin_key_area_pos(begin_key_area_position), end_key_area_pos(end_key_area_position),
          begin_data_pos(begin_data_position), end_data_pos(end_data_position) {}
    KeyPositionType() : key("none") {}
};


class ParserTreeItem {
public:
    enum TextOrChild { TEXT, CHILD };
    // position:
    int row;       // start from o (zero)
    int column;    // start from o (zero)

    // data:
    KeyType key;
    std::vector<std::string> texts;
    std::vector<ParserTreeItem*> childs;
    std::vector<TextOrChild> location_sequence_of_data;

    // methods:
    ParserTreeItem(const KeyType& k, int row_position, int column_position = 0);
    void addText(const std::string& text_part);
    void addChild(ParserTreeItem& item);
};


// TODO: add read keys from file like in html_parser
class ParserTree {
    // data:
protected:
    const std::string& rude_text;
    std::vector<KeyPositionType> key_positions;
    ParserTreeItem* root_item;
    std::set<KeyType> keys;

public:
    std::vector<ParserTreeItem*> last_find;
    std::string error_discription;

    // create / destroy object:
    explicit ParserTree(const std::string& text);
    bool createTree();
    ~ParserTree();

    // main methods:
    ParserTree& find(const KeyType& key);
    std::string outResult() const;

protected:
    // helpful methods:
    bool readKeysFromFile(std::ifstream &fin);
    bool findAllKeyPosition(const std::string& s);
    unsigned int findBeginKeyAreaPosition(const std::string& s, unsigned int begin_pos, const KeyType& key) const;
    unsigned int findBeginDataPosition(const std::string& s, unsigned int begin_key_area_pos, const KeyType& key) const;
    unsigned int findEndDataPosition(const std::string& s, unsigned int begin_data_pos, const KeyType& key) const;
    unsigned int findEndKeyAreaPosition(const std::string& s, unsigned int end_data_pos, const KeyType& key) const;

    typedef std::stack<ParserTreeItem*, std::vector<ParserTreeItem*>> ParserTreeItemStack;
    void SubTree(unsigned int begin_rude_text_pos, unsigned int end_rude_text_position,
                  unsigned int &vector_pos, ParserTreeItem& item);
};
#endif // PARSER_H
