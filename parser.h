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

/// Имя файла со списком ключей
const std::string Key_list_filename = "C:\\Users\\Admin\\Desktop\\parser_test\\tag list.txt";

class ParserTree;


/// Ключ
class KeyType {
public:
    // Новые типы данных:
    /// Тип функции для поиска позиции
    typedef unsigned int (*FindPositionFunctionType)(const std::string& s, unsigned int begin_position, const std::string& key_name, const ParserTree& tree);

    /// Функции поиска позиций
    struct SearchPositionFunctions {
        // Указатели на функции поиска позиций:
        /** Поиск начала ключевой области
         * @param [in] s - строка, в которой выполняется поиск
         * @param [in] begin_pos - позиция, с которой начинается поиск
         * @param [in] tree - дерево, которое вызывает функцию
         * @return позиция начала ключевой области в строке
         */
        unsigned int (*find_begin_key_area_position)(const std::string& s, unsigned int begin_pos, const std::string& key_name, const ParserTree& tree);

        /** Поиск начала данных ключа
         * @param [in] s - строка, в которой выполняется поиск
         * @param [in] begin_key_area_pos - позиция, с которой начинается поиск
         * @param [in] tree - дерево, которое вызывает функцию
         * @return позиция начала данных ключа в строке
         */
        unsigned int (*find_begin_data_position)(const std::string& s, unsigned int begin_key_area_pos, const std::string& key_name, const ParserTree& tree);

        /** Поиск конца данных ключа
         * @param [in] s - строка, в которой выполняется поиск
         * @param [in] begin_data_pos - позиция, с которой начинается поиск
         * @param [in] tree - дерево, которое вызывает функцию
         * @return позиция конца данных ключа в строке
         */
        unsigned int (*find_end_data_position)(const std::string& s, unsigned int begin_data_pos, const std::string& key_name, const ParserTree& tree);

        /** Поиск конца ключевой области
         * @param s - строка, в которой выполняется поиск
         * @param end_data_pos - позиция, с которой начинается поиск
         * @param tree - дерево, которое вызывает функцию
         * @return позиция конца ключевой области в строке
         */
        unsigned int (*find_end_key_area_position)(const std::string& s, unsigned int end_data_pos, const std::string& key_name, const ParserTree& tree);
    };

private:
    /// Имя
    std::string name;
    /// Функции поиска позиций
    SearchPositionFunctions search_position_functions;

public:
    /** Конструктор
     * @param [in] name - имя ключа
     * @param [in] functionFindBeginKeyAreaPosition - функция поиска начала ключевой области
     * @param [in] functionFindBeginDataPosition - функция поиска начала данных ключа
     * @param [in] functionFindEndDataPosition - функция поиска конца данных ключа
     * @param [in] functionFindEndKeyAreaPosition - функция поиска конца ключевой области
     * @sa findBeginKeyAreaPosition, findBeginDataPosition, findEndDataPosition, findEndKeyAreaPosition
     */
    KeyType(const std::string& key_name,
            const FindPositionFunctionType functionFindBeginKeyAreaPosition,
            const FindPositionFunctionType functionFindBeginDataPosition,
            const FindPositionFunctionType functionFindEndDataPosition,
            const FindPositionFunctionType functionFindEndKeyAreaPosition)
        : name(key_name)
    {
        search_position_functions =  {
            functionFindBeginKeyAreaPosition, functionFindBeginDataPosition,
            functionFindEndDataPosition, functionFindEndKeyAreaPosition
        };
    }

    /** Конструктор со структурой
     * @param [in] name - имя ключа
     * @param [in] search_pos_functions - функции поиска позиций (структура)
     */
    KeyType(const std::string& key_name, const SearchPositionFunctions& search_pos_functions)
        : name(key_name), search_position_functions(search_pos_functions) {}

    /** Упрощённый конструктор
     * @param key_name - имя ключа
     */
    KeyType(const std::string& key_name);

    /** Оператор сравнения
     * @param [in] key - ключ, с которым сравниваем
     * @return результат лексикографического сравнения их имён
     */
    bool operator<(KeyType key) const               { return name < key.name; }


    // Чтение полей класса:
    /** Чтение имени ключа
     * @return имя ключа
     */
    const std::string& getName() const              { return name; }

    /** Чтение функции поиска начала ключевой области
     * @return функция поиска начала ключевой области
     */
    const FindPositionFunctionType& getFindBeginKeyAreaPosition() const        { return search_position_functions.find_begin_key_area_position; }

    /** Чтение функции поиска начала данных ключа
     * @return Функция поиска начала данных ключа
     */
    const FindPositionFunctionType& getFindBeginDataPosition() const           { return search_position_functions.find_begin_data_position; }

    /** Чтение функции поиска конца данных ключа
     * @return функция поиска конца данных ключа
     */
    const FindPositionFunctionType& getFindEndDataPosition() const             { return search_position_functions.find_end_data_position; }

    /** Чтение функции поиска конца ключевой области
     * @return функция поиска конца ключевой области
     */
    const FindPositionFunctionType& getFindEndKeyAreaPosition() const          { return search_position_functions.find_end_key_area_position; }
};

/// Функция нахождения search_position_function
const KeyType::SearchPositionFunctions findSearchFunction(const std::string& key_name);


/// Ключ с его местоположением в строке
class KeyPositionType {
public:
    // Новые типы данных:
    /** Местоположение ключа
     * @details Строка, к которой относятся позиции, не указывается
     * Порядок следования позиций:
     * [Начало_ключа[Данные)Конец_ключа)
     * - begin_key_area_pos =  Позиция буквы Н (в слове Начало)
     * - end_key_area_pos   =  Позиция после буквы а (в слове ключа)
     * - begin_data_pos     =  Позиция буквы Д (в слове Данные)
     * - end_data_pos       =  Позиция буквы К (в слове Конец)
     */
    struct Positions
    {
        unsigned int begin_key_area_pos, end_key_area_pos;  ///< Начало и конец зоны действия ключа
        unsigned int begin_data_pos, end_data_pos;          ///< Начало и конец данных ключа
    };

private:
    // Данные:
    KeyType key;            ///< Ключ
    Positions positions;    ///< Местоположение ключа

public:
    // Конструкторы:
    /** Конструктор
     * @param [in] k - ключ
     * @param [in] begin_key_area_position - позиция начала зоны действия ключа
     * @param [in] end_key_area_position - позиция конца зоны действия ключа
     * @param [in] begin_data_position - позиция начала данных ключа
     * @param [in] end_data_position - позиция конца данных ключа
     */
    KeyPositionType(const KeyType& k, unsigned int begin_key_area_position, unsigned int end_key_area_position,
                    unsigned int begin_data_position, unsigned int end_data_position)
        : key(k)
    {
        positions = {
            begin_key_area_position, end_key_area_position,
            begin_data_position, end_data_position };
    }

    /** Конструктор со структурой
     * @param k - ключ
     * @param poss - местоположение ключа
     */
    KeyPositionType(const KeyType& k, const Positions& poss) : key(k), positions(poss) {}

    /** Упрощённый конструктор
     * @details Любая позиция ключа = 0
     * @param k - ключ
     */
    explicit KeyPositionType(KeyType k) : key(k) { positions = {}; }

    // Чтение полей класса:
    /** Чтение ключа
     * @return ключ
     */
    const KeyType& getKey() const                     { return key; }

    /** Чтение позиции начала зоны действия ключа
     * @return Позиция начала зоны действия ключа
     */
    unsigned int getBeginKeyAreaPosition() const      { return positions.begin_key_area_pos; }

    /** Чтение позиции конца зоны действия ключа
     * @return Позиция конца зоны действия ключа
     */
    unsigned int getEndKeyAreaPosition() const        { return positions.end_key_area_pos; }

    /** Чтение позиции начала данных ключа
     * @return Позиция начала данных ключа
     */
    unsigned int getBeginDataPosition() const         { return positions.begin_data_pos; }

    /** Чтение позиции конца данных ключа
     * @return Позиция конца данных ключа
     */
    unsigned int getEndDataPosition() const           { return positions.end_data_pos; }

    // Установка полей класса:
    /** Установка ключа
     * @param [in] new_key - новый ключ
     */
    void setKey(const KeyType& new_key)                       { key = new_key; }

    /** Установка позиции начала зоны действия ключа
     * @param [in] new_position -  Новая позиция начала зоны действия ключа
     */
    void setBeginKeyAreaPosition(unsigned int new_position)   { positions.begin_key_area_pos = new_position; }

    /** Установка позиции конца зоны действия ключа
     * @param [in] new_position - Новая позиция конца зоны действия ключа
     */
    void setEndKeyAreaPosition(unsigned int new_position)     { positions.end_key_area_pos = new_position; }

    /** Установка позиции начала данных ключа
     * @param [in] new_position - Новая позиция начала данных ключа
     */
    void setBeginDataPosition(unsigned int new_position)      { positions.begin_data_pos = new_position; }

    /** Установка позиции конца данных ключа
     * @param [in] new_position - Новая позиция конца данных ключа
     */
    void setEndDataPosition(unsigned int new_position)        { positions.end_data_pos = new_position; }
};


/// Узел дерева ParserTreeItem
class ParserTreeItem {
public:
    // Новые типы данных:
    /** Перечисление для определения последовательности вхождений
     * @details Содержит константы, определяющие последовательность вхождений
     *  ключей-текста в location_sequence_of_data
     *
     * @value TEXT Текст
     * @value CHILD Дочерний узел (вложенный ключ)
     */
    enum TextOrChild { TEXT, CHILD };

private:
    // Данные:
    KeyType key;                          ///< Ключ
    std::vector<std::string> texts;       ///< Вектор текстовых данных, не содержащих ключи
    std::vector<ParserTreeItem*> childs;                  ///< Вектор дочерних узлов
    /** Последовательность вхождений
     * @details Вектор, содержащий последовательность вхождений текстовых отрывков и других ключей, находящиеся в
     * области данных текущего ключа
     */
    std::vector<TextOrChild> location_sequence_of_data;

    // Позиция:
    int row;       /**< Ряд
     * @details "Глубина" дерева, начинается с нуля
     */
    int column;    /**< Колонна
     * @details Номер узла относительно предка, начинается с нуля
     */

public:
    // Методы:
    /** Конструктор
     * @param [in] k - ключ
     * @param [in] row_position - ряд
     * @param [in] column_position - колонна
     */
    ParserTreeItem(const KeyType& k, int row_position, int column_position = 0);

    // Чтение полей класса:
    /** Чтение ключа
     * @return ключ
     */
    const KeyType& getKey() const;

    /** Чтение вектора текстовых данных, не содержащих ключи
     * @return вектор текстовых данных, не содержащих ключи
     */
    const std::vector<std::string>& getTexts() const;

    /** Чтение вектора дочерних узлов
     * @return вектор дочерних узлов
     */
    const std::vector<ParserTreeItem*>& getChilds() const;

    /** Чтение вектора последовательности вхождений
     * @return вектор последовательности вхождений
     */
    const std::vector<TextOrChild>& getLocationSequenceOfData() const;

    /** Чтение ряда узла
     * @return ряд узла
     */
    int getRow() const;

    /** Чтение колонны узла
     * @return колонна узла
     */
    int getColumn() const;

    // Установка полей класса:
    /** Добавления текста, не содержащего ключи
     * @param [in] text_part - текст, не содержащий ключи
     */
    void addText(const std::string& text_part);

    /** Добавление дочернего узла (вложенного ключа)
     * @param [in] item - дочерний узел
     */
    void addChild(ParserTreeItem *item);

    /** Удаление последнего добавленного текста, не содержащего ключи
     */
    void deleteLastText();

    /** Удаление последнего добавленного дочернего узла
     * @details Удаляет всю ветку дочернего узла
     */
    void deleteLastChild();
};


// TODO: Задокументировать
class ParserTree {
public:
    // Новые типы данных:
    typedef std::stack<ParserTreeItem*, std::vector<ParserTreeItem*>> ParserTreeItemStack;

private:
    // Данные:
    std::string rude_text;          ///< Исходный текст
    std::vector<KeyPositionType> key_positions;  ///< Вектор местоположений ключей
    ParserTreeItem* root_item;      ///< Коренной узел дерева
    std::set<KeyType> keys;         ///< Множество ключей
    std::string error_description;  ///< Описание текущих ошибок
    std::vector<ParserTreeItem*> last_find;  ///< Результат посдеднего поиска ключей (узлов)

public:
    // Создать / уничтожить дерево:
    /** Конструктор
     * @param text - исходный текст
     */
    explicit ParserTree(const std::string& text);

    /** Сконструировать дерево
     * @return Удалось ли создать дерево
     * @note В случае неудачи конструирования дерева причину ошибки можно узнать при помощи getErrorDescription()
     */
    bool createTree();

    /** Деструктор
     */
    ~ParserTree();


    // Основные методы:
    /** Считывание ключей с файла
     * @param fin - файл
     * @return успешность считывания ключей с файла
     */
    bool readKeysFromFile(std::ifstream &fin);

    /** Поиск ключа
     * @param key - ключ, который необходимо найти
     * @return Вызывающий объект
     */
    ParserTree& find(const KeyType& key);

    /** Вывод дерева через интерфейс ASCII
     * @return строка с деревом в ASCII представление
     */
    std::string outASCIITree() const;


    // Чтение полей класса:
    /** Чтение исходного текста
     * @return исходный текст
     */
    const std::string& getRudeText() const;

    /** Чтение текущих ошибок
     * @return список установленных ошибок
     */
    const std::string& getErrorDescription() const;

protected:
    // TODO: Зодокументировать
    // Вспомогательные методы:
    bool findAllKeyPosition(const std::string& s);    
    void SubTree(unsigned int begin_rude_text_pos, unsigned int end_rude_text_position,
                  unsigned int &vector_pos, ParserTreeItem& item);
};
#endif // PARSER_H
