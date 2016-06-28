#include "parser.h"

/* Helpful for output function; locale for parser.cpp */
static void writeWithIndention(std::stringstream &sstream, const std::string& indent, int count_indent, const std::string& s);


/* === ParserTreeItem === */
ParserTreeItem::ParserTreeItem(const KeyType& k, int row_position, int column_position)
    : row(row_position), column(column_position), key(k)
{
}

void ParserTreeItem::addText(const std::string& text_part)
{
    location_sequence_of_data.push_back(TEXT);
    texts.push_back(text_part);
}

void ParserTreeItem::addChild(ParserTreeItem& item)
{
    location_sequence_of_data.push_back(CHILD);
    childs.push_back(&item);
}

//==========================================================

/* === ParserTree === */

/* Create and destroy tree; Public */
ParserTree::ParserTree(const std::string& text)
    : rude_text(text), root_item(new ParserTreeItem(KeyType(""), 0, 0)), error_discription()
{
    /* Check text fo empty */
    if (rude_text.empty())
        error_discription += "Input text is empty;\n";

    /* Read keys from file */
    std::ifstream fin(Tag_list_filename.c_str());
    if (!fin.is_open())
    {
        error_discription += "File with tag list can't be opened;\n";
        return;
    }
    readKeysFromFile(fin);
    fin.close();

    /* Find all keys postition and put result to key_positions
     *   with check for text correctness, sort if needed */
    bool saccess;
    saccess = findAllKeyPosition(rude_text);
    if (saccess == false)
        error_discription += "Input text can't be parsing to tree;\n";

    auto sort_compare = [](KeyPositionType a, KeyPositionType b) { return a.begin_key_area_pos < b.begin_key_area_pos; };
    if (saccess == true && !std::is_sorted(key_positions.cbegin(), key_positions.cend(), sort_compare))
        std::sort(key_positions.begin(), key_positions.end(), sort_compare);
}


// TODO: Describe errors
bool ParserTree::createTree()
{
    /* Check errors, that necessary */
    if (error_discription.empty() == false)
        return false;

    /* Create tree by recursive function SubTree */
    try
    {
        unsigned int vector_position = 0;
        SubTree(0, rude_text.size(), vector_position, *root_item);
    }
    catch (std::bad_alloc)
    {
        return false;
    }

    return true;
}


ParserTree::~ParserTree()
{
    ParserTreeItem* pItem = root_item;
    ParserTreeItemStack stack;
    while(!pItem->childs.empty())
    {
        stack.push(pItem);
        pItem = pItem->childs[pItem->childs.size() - 1];
    }
    stack.push(pItem);

    while(stack.size() != 1)    // on top of stack always most column-right
    {                           // != 1 else pItem->child reference to hollowness and .pop_back() lead to error
        delete stack.top();
        stack.pop();
        pItem = stack.top();
        pItem->childs.pop_back();

        while (!pItem->childs.empty())
        {
            pItem = pItem->childs[pItem->childs.size() - 1];
            stack.push(pItem);
        }
    }
    delete stack.top();
}


/* Find items; Public */
// TODO: ParserTree::find()
ParserTree& ParserTree::find(const KeyType& key)
{
    if (key.name != "")
        return *this;
    return *this;
}


/* Create sub trees (recursive function); Protected */
void ParserTree::SubTree(unsigned int begin_rude_text_pos, unsigned int end_rude_text_pos,
                           unsigned int& vector_pos, ParserTreeItem& item)
{
    static std::stack<unsigned int, std::vector<unsigned int>> vector_pos_stack;
    unsigned int text_pos = begin_rude_text_pos;        // position in rude_text
                                                 // vector_pos = (max - 1) number used key position in key_positions
    while (text_pos != end_rude_text_pos)
    {
        /* Add new item */
        if (vector_pos < key_positions.size() && text_pos == key_positions[vector_pos].begin_key_area_pos)
        {
            ParserTreeItem * p_child = new ParserTreeItem(KeyType(rude_text.substr(text_pos, key_positions[vector_pos].begin_data_pos - text_pos)),
                                                                             item.row + 1, item.childs.size());
            item.addChild(*p_child);
            vector_pos_stack.push(vector_pos);
            vector_pos++;
            SubTree(key_positions[vector_pos - 1].begin_data_pos, key_positions[vector_pos - 1].end_data_pos, vector_pos, *p_child);
            text_pos = key_positions[vector_pos_stack.top()].end_key_area_pos;
            vector_pos_stack.pop();
        }
        /* Add new text */
        else
        {
            unsigned int end_temp_pos;
            if (vector_pos < key_positions.size() && key_positions[vector_pos].begin_key_area_pos < end_rude_text_pos)
                end_temp_pos = key_positions[vector_pos].begin_key_area_pos;
            else
                end_temp_pos = end_rude_text_pos;
            item.addText(rude_text.substr(text_pos, end_temp_pos - text_pos));
            text_pos = end_temp_pos;
        }
    }
}


/* Set keys (set) from file; protected */
bool ParserTree::readKeysFromFile(std::ifstream& fin)
{
    std::string cur_str;
    std::getline(fin, cur_str);
    while (fin)
    {
        cur_str += '\n';
        /* Skip string start from '#' */
        if (cur_str[0] == '#')
        {
            std::getline(fin, cur_str);
            continue;
        }
        /* Skip emty string */
        if (cur_str.find_first_not_of(" \t\n") == std::string::npos)
        {
            std::getline(fin, cur_str);
            continue;
        }

        /* Set keys */
        unsigned int  begin_key_word = cur_str.find_first_not_of(" \t\n");
        unsigned int end_key_word =  begin_key_word;
        std::string key_word;
        while ( begin_key_word < cur_str.size())
        {
            end_key_word = cur_str.find_first_of(" \t\n",  begin_key_word);
            key_word = std::move(cur_str.substr( begin_key_word, end_key_word -  begin_key_word));
            keys.insert(KeyType("<" + key_word + "> </" + key_word + ">"));
            begin_key_word = cur_str.find_first_not_of(" \t\n", end_key_word + 1);
        }

        std::getline(fin, cur_str);
    }

    return true;
}


/* Set key_positions (all possible positions); Protected */
// NOTE: May be optimized
// TODO: Read keys from file
bool ParserTree::findAllKeyPosition(const std::string& s)
{
    unsigned int begin_key_area_pos, end_key_area_pos;  // [...)
    unsigned int begin_data_pos, end_data_pos;          // [...)
    unsigned int find_current_pos, find_end_pos;

    find_current_pos = 0;
    find_end_pos = s.size();
    for (KeyType current_key : keys)
    {
        while (find_current_pos != find_end_pos)
        {
            begin_key_area_pos = findBeginKeyAreaPosition(s, find_current_pos, current_key);
            if (begin_key_area_pos == std::string::npos)
                break;
            begin_data_pos = findBeginDataPosition(s, begin_key_area_pos, current_key);
            if (begin_data_pos == std::string::npos)
                return false;
            end_data_pos = findEndDataPosition(s, begin_data_pos, current_key);
            if (end_data_pos == std::string::npos)
                return false;
            end_key_area_pos = findEndKeyAreaPosition(s, end_data_pos, current_key);
            if (end_key_area_pos == std::string::npos)
                return false;

            key_positions.push_back(KeyPositionType(current_key, begin_key_area_pos, end_key_area_pos,
                                                   begin_data_pos, end_data_pos));
            find_current_pos = begin_data_pos;
        }
        find_current_pos = 0;
    }
    return true;
}

/* Helpful function to find keywords */
unsigned int ParserTree::findBeginKeyAreaPosition(const std::string& s, unsigned int begin_pos, const KeyType& key) const
{
    unsigned int whitespace_pos = key.name.find(' ');
    return s.find(key.name.substr(0, whitespace_pos - 1), begin_pos);
}

unsigned int ParserTree::findBeginDataPosition(const std::string& s, unsigned int begin_key_area_pos, const KeyType& key) const
{
    return s.find('>', begin_key_area_pos) + 1;
}

unsigned int ParserTree::findEndDataPosition(const std::string& s, unsigned int begin_data_pos, const KeyType& key) const
{
    int count_nested_same_name_keys = 0;
    unsigned int whitespace_pos = key.name.find(' ');
    const std::string end_key_str = key.name.substr(whitespace_pos + 1);
    unsigned int find_begin, find_end;
    unsigned int end_key_word;

    find_begin = findBeginKeyAreaPosition(s, begin_data_pos, key);
    find_end = s.find(end_key_str, begin_data_pos);

    /* Treat nested keys */
    while (find_begin < find_end)
    {
        count_nested_same_name_keys++;
        /* Find next pair start_key - finish_key */
        end_key_word = findBeginDataPosition(s, find_begin, key);
        find_begin = findBeginKeyAreaPosition(s, end_key_word, key);
        end_key_word = findEndKeyAreaPosition(s, find_end, key);
        find_end = s.find(end_key_str, end_key_word);
    }

    return find_end;
}

unsigned int ParserTree::findEndKeyAreaPosition(const std::string& s, unsigned int end_data_pos, const KeyType& key) const
{
    return s.find('>', end_data_pos) + 1;
}


/* Show methods */
std::string ParserTree::outResult() const
{
    struct ItemOutputState
    {
        unsigned int location_sequence_num;
        unsigned int text_num;
        unsigned int child_num;
    };
    ItemOutputState zero_state = {};

    const std::string indent("|   ");
    typedef std::pair<ParserTreeItem*, ItemOutputState> ItemAndState;
    std::stack<ItemAndState, std::vector<ItemAndState>> stack;

    const ParserTreeItem* pItem;
    ItemOutputState* pState;
    std::stringstream sstream;

    stack.push(ItemAndState(root_item, zero_state));
    pItem = stack.top().first;
    pState = &stack.top().second;

    while (!stack.empty())
    {
        /* New key area */
        if (pState->location_sequence_num == 0)
        {
            writeWithIndention(sstream, indent, pItem->row, std::string("@" + pItem->key.name + "\n"));
            writeWithIndention(sstream, indent, pItem->row, "//====================\n");
        }

        // 'end key area' place immediatly after 'new key area' in case of empty key data

        /* End key area */
        if (pState->location_sequence_num == pItem->location_sequence_of_data.size())
        {
            writeWithIndention(sstream, indent, pItem->row, "\\\\====================\n");
            stack.pop();
            if (stack.empty() == false)
            {
                pItem = stack.top().first;
                pState = &stack.top().second;
            }
            continue;
        }

        /* Key data */
        if (pItem->location_sequence_of_data.empty() == false)
        {
            if (pItem->location_sequence_of_data[pState->location_sequence_num] == ParserTreeItem::TEXT)
            {
                writeWithIndention(sstream, indent, pItem->row + 1, "[" + pItem->texts[pState->text_num]);
                if (pItem->texts[pState->text_num].back() == '\n')
                    writeWithIndention(sstream, indent, pItem->row + 1, "]\n");
                else
                    sstream << "]\n";
                pState->text_num++;
                pState->location_sequence_num++;
            }
            else
            {
                pState->child_num++;
                pState->location_sequence_num++;
                stack.push(ItemAndState(pItem->childs[pState->child_num - 1], zero_state));
            }
        }

        /* Set last item-state */
        pItem = stack.top().first;
        pState = &stack.top().second;
    }

    return sstream.str();
}


//=======================================================

/* === Other funtion === */
void writeWithIndention(std::stringstream &sstream, const std::string& indent, int count_indent, const std::string& s)
{
    unsigned int last_pos = -1;
    unsigned int prev_pos;
    while (last_pos + 1 < s.size())      // write line by line
    {
        prev_pos = last_pos + 1;         // eat '\n'
        last_pos = s.find('\n', prev_pos);
        if (last_pos == std::string::npos)
            last_pos = s.size();

        for (int i = 0; i < count_indent; i++)
            sstream << indent;
        for (unsigned int pos = prev_pos; pos <= last_pos; pos++)
            sstream.put(s[pos]);
    }
}
