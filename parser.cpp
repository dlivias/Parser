#include "parser.h"

/** Предваряет строку отступом
 * @details  Вспомогательная локальная функция для вывода дерева в ASCII интерфейсе
 * Записывает в строку output строку s, предваряя её отступом indent count_indent раз
 *
 * @param [out] output - результирующая строка с отступом
 * @param [in]  indent - отступ (любой набор символов)
 * @param [in]  count_indent - количество отступов перед строкой
 * @param [in]  s - строка, которая будет предварена отступами
 */
static void writeWithIndention(std::string& output, const std::string& indent, int count_indent, const std::string& s);


/* === ParserTreeItem === */
// Конструктор:
ParserTreeItem::ParserTreeItem(const KeyType& k, int row_position, int column_position)
    : key(k), row(row_position), column(column_position)
{
}

// Чтение полей класса:
const KeyType& ParserTreeItem::getKey() const                             { return key; }
const std::vector<std::string>& ParserTreeItem::getTexts() const          { return texts; }
const std::vector<ParserTreeItem*>& ParserTreeItem::getChilds() const     { return childs; }
const std::vector<ParserTreeItem::TextOrChild>& ParserTreeItem::getLocationSequenceOfData() const
{
    return location_sequence_of_data;
}
int ParserTreeItem::getRow() const      { return row; }
int ParserTreeItem::getColumn() const   { return column; }


// Установка полей класса:
void ParserTreeItem::addText(const std::string& text_part)
{
    location_sequence_of_data.push_back(TEXT);
    texts.push_back(text_part);
}

// NOTE: Почему нельзя передать const ParserTreeItem * item
void ParserTreeItem::addChild(ParserTreeItem *item)
{
    location_sequence_of_data.push_back(CHILD);
    childs.push_back(item);
}

void ParserTreeItem::deleteLastText()
{
    location_sequence_of_data.pop_back();
    texts.pop_back();
}

void ParserTreeItem::deleteLastChild()
{
    ParserTreeItem* p_item = this->childs.back();
    ParserTree::ParserTreeItemStack stack;

    /* Удаление дочерних элементов требуемого узла: */

    /* Доходим до крайнего правого нижнего узла,
        записывая все промежуточные узлы в стек */
    while(!p_item->childs.empty())
    {
        stack.push(p_item);
        p_item = p_item->childs.back();
    }
    stack.push(p_item);

    /* Последовательно удаляем элементы, двигаясь от правых веток к левым */
    while(stack.size() != 1)    // На вершине стеканаходится самый правый нижний узел
    {                           // != 1 иначе p_item = stack.top() (после stack.pop()) мокажет указывать на пустоту
        /* Удаляем самый правый нижний элемент
            из памяти, стэка и вектора childs */
        delete stack.top();
        stack.pop();
        p_item = stack.top();
        p_item->childs.pop_back();

        /* Спускаемся на новый самый правый нижний узел. Промежуточные узлы в стеке */
        while (!p_item->childs.empty())
        {
            p_item = p_item->childs.back();
            stack.push(p_item);
        }
    }

    /* Удаление требуемого узла: */
    delete stack.top();
    stack.pop();
    if (this->location_sequence_of_data.back() == TEXT)
    {
        this->location_sequence_of_data.pop_back();
        this->location_sequence_of_data.back() = TEXT;
    }
    else
        this->location_sequence_of_data.pop_back();
    this->childs.pop_back();
}


//==========================================================

/* === ParserTree === */

// Конструктор:
ParserTree::ParserTree(const std::string& text)
    : rude_text(text), root_item(new ParserTreeItem(KeyType(""), 0, 0)), error_description()
{
    /* Проверка на пустую строку */
    if (rude_text.empty())
        error_description += "Input text is empty;\n";

    /* Считываем список ключей с файла */
    std::ifstream fin(Key_list_filename);
    if (!fin.is_open())
    {
        error_description += "File with tag list can't be opened;\n";
        return;
    }
    readKeysFromFile(fin);
    fin.close();

    /* Находим все позиции ключей, результат в векторе key_positions */
    bool saccess;
    saccess = findAllKeyPosition(rude_text);
    if (saccess == false)
        error_description += "Input text can't be parsing to tree;\n";

    /* Если необходимо - сортируем ключи в порядки появления в тексте */
    auto sort_compare = [](KeyPositionType a, KeyPositionType b)
    {
        return a.getBeginKeyAreaPosition() < b.getBeginKeyAreaPosition();
    };
    if (saccess == true && !std::is_sorted(key_positions.cbegin(), key_positions.cend(), sort_compare))
        std::sort(key_positions.begin(), key_positions.end(), sort_compare);
}


// Создать дерево:
// TODO: Описать ошибки (строка, которую не обработать)
bool ParserTree::createTree()
{
    /* Проверка на ошибки, созданные в конструкторе */
    if (error_description.empty() == false)
        return false;

    /* Создаем дерево рекурсивной функцией SubTree */
    try
    {
        unsigned int vector_position = 0;
        SubTree(0, rude_text.size(), vector_position, *root_item);
    }
    catch (std::bad_alloc)
    {
        error_description += "Not enough memory;\n";
        return false;
    }

    return true;
}


// Деструктор:
ParserTree::~ParserTree()
{
    while (root_item->getChilds().empty() == false)
        root_item->deleteLastChild();
    delete root_item;
}


// Поиск данных по ключу:
// TODO: ParserTree::find()
ParserTree& ParserTree::find(const KeyType& key)
{
    if (key.getName() != "")
        return *this;
    return *this;
}


// Вспомоготельные функции:
// Создаем поддерево (рекурсивно):
void ParserTree::SubTree(unsigned int begin_rude_text_pos, unsigned int end_rude_text_pos,
                           unsigned int& vector_pos, ParserTreeItem& item)
{
    static std::stack<unsigned int, std::vector<unsigned int>> vector_pos_stack;
    unsigned int text_pos = begin_rude_text_pos;        // position in rude_text
                                                 // vector_pos = (max - 1) number used key position in key_positions
    while (text_pos != end_rude_text_pos)
    {
        /* Add new item */
        if (vector_pos < key_positions.size() && text_pos == key_positions[vector_pos].getBeginKeyAreaPosition())
        {
            ParserTreeItem * p_child = new ParserTreeItem(
                        KeyType(rude_text.substr(text_pos, key_positions[vector_pos].getBeginDataPosition() - text_pos)), item.getRow() + 1, item.getChilds().size());
            item.addChild(p_child);
            vector_pos_stack.push(vector_pos);
            vector_pos++;
            SubTree(key_positions[vector_pos - 1].getBeginDataPosition(), key_positions[vector_pos - 1].getEndDataPosition(), vector_pos, *p_child);
            text_pos = key_positions[vector_pos_stack.top()].getEndKeyAreaPosition();
            vector_pos_stack.pop();
        }
        /* Add new text */
        else
        {
            unsigned int end_temp_pos;
            if (vector_pos < key_positions.size() && key_positions[vector_pos].getBeginKeyAreaPosition() < end_rude_text_pos)
                end_temp_pos = key_positions[vector_pos].getBeginKeyAreaPosition();
            else
                end_temp_pos = end_rude_text_pos;
            item.addText(rude_text.substr(text_pos, end_temp_pos - text_pos));
            text_pos = end_temp_pos;
        }
    }
}


// Считать список ключей с файла:
bool ParserTree::readKeysFromFile(std::ifstream& fin)
{
    /* Проверка на успешность открытия файла */
    if (!fin.is_open())
        return false;

    /* Считывание строк с файла */
    std::string cur_str;
    std::getline(fin, cur_str);
    while (fin)
    {
        cur_str += '\n';       // Восстановление исходной строки (как в файле)
        /* Пропускаем строки, начинающиеся с '#' */
        if (cur_str[0] == '#')
        {
            std::getline(fin, cur_str);
            continue;
        }
        /* Пропускаем пустые строки */
        if (cur_str.find_first_not_of(" \t\n") == std::string::npos)
        {
            std::getline(fin, cur_str);
            continue;
        }

        /* Устанавливаем ключи */
        unsigned int  begin_key_word = cur_str.find_first_not_of(" \t\n");
        unsigned int end_key_word;
        std::string key_word;
        while ( begin_key_word < cur_str.size())
        {
            end_key_word = cur_str.find_first_of(" \t\n",  begin_key_word);
            key_word = std::move(cur_str.substr( begin_key_word, end_key_word -  begin_key_word));
            keys.insert(KeyType("<" + key_word + "> </" + key_word + ">"));
            begin_key_word = cur_str.find_first_not_of(" \t\n", end_key_word + 1);
        }

        /* Считываем следующую строку */
        std::getline(fin, cur_str);
    }

    return true;
}


/* Set key_positions (all possible positions); Protected */
// NOTE: May be optimized
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
    unsigned int whitespace_pos = key.getName().find(' ');
    const std::string& key_word = key.getName().substr(0, whitespace_pos - 1);
    unsigned int find1 = s.find(key_word + '>', begin_pos);
    unsigned int find2 = s.find(key_word + ' ', begin_pos);
    return find1 < find2 ? find1 : find2;
}

unsigned int ParserTree::findBeginDataPosition(const std::string& s, unsigned int begin_key_area_pos, const KeyType& key) const
{
    return s.find('>', begin_key_area_pos) + 1;
}

unsigned int ParserTree::findEndDataPosition(const std::string& s, unsigned int begin_data_pos, const KeyType& key) const
{
    int count_nested_same_name_keys = 0;
    unsigned int whitespace_pos = key.getName().find(' ');
    const std::string end_key_str = key.getName().substr(whitespace_pos + 1);
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
    //return s.find(key.name.substr(key.name.find(' ')), begin_data_pos);
}

unsigned int ParserTree::findEndKeyAreaPosition(const std::string& s, unsigned int end_data_pos, const KeyType& key) const
{
    return s.find('>', end_data_pos) + 1;
}


/* Show methods */
std::string ParserTree::outASCIITree() const
{
    if (rude_text.empty())
        return "";

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

    const ParserTreeItem* p_item;
    ItemOutputState* p_state;
    std::string output;

    stack.push(ItemAndState(root_item, zero_state));
    p_item = stack.top().first;
    p_state = &stack.top().second;

    while (!stack.empty())
    {
        /* New key area */
        if (p_state->location_sequence_num == 0)
        {
            writeWithIndention(output, indent, p_item->getRow(), std::string("@" + p_item->getKey().getName() + "\n"));
            writeWithIndention(output, indent, p_item->getRow(), "//====================\n");
        }

        // 'end key area' place immediatly after 'new key area' in case of empty key data

        /* End key area */
        if (p_state->location_sequence_num == p_item->getLocationSequenceOfData().size())
        {
            writeWithIndention(output, indent, p_item->getRow(), "\\\\====================\n");
            stack.pop();
            if (stack.empty() == false)
            {
                p_item = stack.top().first;
                p_state = &stack.top().second;
            }
            continue;
        }

        /* Key data */
        if (p_item->getLocationSequenceOfData().empty() == false)
        {
            if (p_item->getLocationSequenceOfData()[p_state->location_sequence_num] == ParserTreeItem::TEXT)
            {
                writeWithIndention(output, indent, p_item->getRow() + 1, "[" + p_item->getTexts()[p_state->text_num]);
                if (p_item->getTexts()[p_state->text_num].back() == '\n')
                    writeWithIndention(output, indent, p_item->getRow() + 1, "]\n");
                else
                    output += "]\n";
                p_state->text_num++;
                p_state->location_sequence_num++;
            }
            else
            {
                p_state->child_num++;
                p_state->location_sequence_num++;
                stack.push(ItemAndState(p_item->getChilds()[p_state->child_num - 1], zero_state));
            }
        }

        /* Set last item-state */
        p_item = stack.top().first;
        p_state = &stack.top().second;
    }

    return output;
}


// Чтение полей класса:
const std::string& ParserTree::getRudeText() const              { return rude_text; }
const std::string& ParserTree::getErrorDescription() const      { return error_description; }


//=======================================================

/* === Other funtion === */
// TODO: Заменить поледний for на std::copy
void writeWithIndention(std::string &output, const std::string& indent, int count_indent, const std::string& s)
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
            output += indent;
        for (unsigned int pos = prev_pos; pos <= last_pos; pos++)
            output += s[pos];
    }
}
