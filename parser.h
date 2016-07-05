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


/// Ключ
class KeyType {
private:
    /// Имя
    std::string name;
public:
    /** Конструктор
     * @param [in] name - Имя ключа
     */
    explicit KeyType(const std::string& key_name)   : name(key_name) {}

    /** Оператор сравнения
     * @param [in] key - ключ, с которым сравниваем
     * @return результат лексикографического сравнения их имён
     */
    bool operator<(KeyType key) const               { return name < key.name; }

    /** Чтение имени ключа
     * @return имя ключа
     */
    const std::string& getName() const              { return name; }
};


/// Ключ с его местоположением в строке
class KeyPositionType {
private:
    // Данные:
    /// Ключ
    KeyType key;
    /** Местоположение ключа
     * @details Строка, к которой относятся позиции, не указывается
     * Порядок следования позиций:
     * [Начало_ключа[Данные)Конец_ключа)
     * - begin_key_area_pos =  Позиция буквы Н (в слове Начало)
     * - end_key_area_pos   =  Позиция после буквы а (в слове ключа)
     * - begin_data_pos     =  Позиция буквы Д (в слове Данные)
     * - end_data_pos       =  Позиция буквы К (в слове Конец)
     */
    unsigned int begin_key_area_pos, end_key_area_pos;  ///< Начало и конец зоны действия ключа
    unsigned int begin_data_pos, end_data_pos;          ///< Начало и конец данных ключа

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
        : key(k), begin_key_area_pos(begin_key_area_position), end_key_area_pos(end_key_area_position),
          begin_data_pos(begin_data_position), end_data_pos(end_data_position) {}

    /** Базовый конструктор
     * @details Имя ключа = none;
     * Любая позиция ключа = 0;
     */
    KeyPositionType() : key("none") {}

    // Чтение полей класса:
    /** Чтение ключа
     * @return ключ
     */
    const KeyType& getKey() const                     { return key; }

    /** Чтение позиции начала зоны действия ключа
     * @return Позиция начала зоны действия ключа
     */
    unsigned int getBeginKeyAreaPosition() const      { return begin_key_area_pos; }

    /** Чтение позиции конца зоны действия ключа
     * @return Позиция конца зоны действия ключа
     */
    unsigned int getEndKeyAreaPosition() const        { return end_key_area_pos; }

    /** Чтение позиции начала данных ключа
     * @return Позиция начала данных ключа
     */
    unsigned int getBeginDataPosition() const         { return begin_data_pos; }

    /** Чтение позиции конца данных ключа
     * @return Позиция конца данных ключа
     */
    unsigned int getEndDataPosition() const           { return end_data_pos; }

    // Установка полей класса:
    /** Установка ключа
     * @param [in] new_key - новый ключ
     */
    void setKey(const KeyType& new_key)                       { key = new_key; }

    /** Установка позиции начала зоны действия ключа
     * @param [in] new_position -  Новая позиция начала зоны действия ключа
     */
    void setBeginKeyAreaPosition(unsigned int new_position)   { begin_key_area_pos = new_position; }

    /** Установка позиции конца зоны действия ключа
     * @param [in] new_position - Новая позиция конца зоны действия ключа
     */
    void setEndKeyAreaPosition(unsigned int new_position)     { end_key_area_pos = new_position; }

    /** Установка позиции начала данных ключа
     * @param [in] new_position - Новая позиция начала данных ключа
     */
    void setBeginDataPosition(unsigned int new_position)      { begin_data_pos = new_position; }

    /** Установка позиции конца данных ключа
     * @param [in] new_position - Новая позиция конца данных ключа
     */
    void setEndDataPosition(unsigned int new_position)        { end_data_pos = new_position; }
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
    unsigned int findBeginKeyAreaPosition(const std::string& s, unsigned int begin_pos, const KeyType& key) const;
    unsigned int findBeginDataPosition(const std::string& s, unsigned int begin_key_area_pos, const KeyType& key) const;
    unsigned int findEndDataPosition(const std::string& s, unsigned int begin_data_pos, const KeyType& key) const;
    unsigned int findEndKeyAreaPosition(const std::string& s, unsigned int end_data_pos, const KeyType& key) const;

    void SubTree(unsigned int begin_rude_text_pos, unsigned int end_rude_text_position,
                  unsigned int &vector_pos, ParserTreeItem& item);
};
#endif // PARSER_H
