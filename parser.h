#ifndef PARSER_H
#define PARSER_H

#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <stack>
#include <algorithm>
#include <iterator>

using std::string;
using std::vector;
using std::set;

class KeyType {
public:
    string name;
    bool operator<(KeyType key) const { return name < key.name; }
    explicit KeyType(const string& key_name) : name(key_name) {}
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
    vector<string> texts;
    vector<ParserTreeItem*> childs;
    vector<TextOrChild> location_sequence_of_data;

    // methods:
    ParserTreeItem(KeyType k, int row_position, int column_position = 0);
    void addText(const string& text_part);
    void addChild(ParserTreeItem &item);
};


class ParserTree {
    // data:
private:
    string rude_text;
    vector<KeyPositionType> key_positions;
    ParserTreeItem* root_item;

public:
    vector<ParserTreeItem*> last_find;

    // create / destroy object:
    explicit ParserTree(const string& text);
    bool createTree();
    ~ParserTree();

    // main methods:
    ParserTree& find(const KeyType& key);
    string outResult() const;

protected:
    // helpful methods:
    bool findAllKeyPosition(const string& s);
    unsigned int findBeginKeyAreaPosition(const string& s, unsigned int begin_pos, const KeyType& key) const;
    unsigned int findBeginDataPosition(const string& s, unsigned int begin_key_area_pos, const KeyType& key) const;
    unsigned int findEndDataPosition(const string& s, unsigned int begin_data_pos, const KeyType& key) const;
    unsigned int findEndKeyAreaPosition(const string& s, unsigned int end_data_pos, const KeyType& key) const;

    typedef std::stack<ParserTreeItem*, vector<ParserTreeItem*>> ParserTreeItemStack;
    void SubTree(unsigned int begin_rude_text_pos, unsigned int end_rude_text_position,
                  unsigned int &vector_pos, ParserTreeItem& item);
};
#endif // PARSER_H
