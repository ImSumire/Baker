#pragma once

#include <iostream>
#include <unistd.h>
#include <termios.h>

#include "log.hpp"
#include "utils.hpp"

#define KEY_UP 65
#define KEY_DOWN 66

void eraseLines(int count) {
    // https://stackoverflow.com/questions/61919292
    if (count > 0) {
        std::cout << "\x1b[2K";  // Delete current line
        for (int i = 1; i < count; i++) {
            std::cout << "\x1b[1A" << "\x1b[2K";  // Move cursor up and delete line
        }
        std::cout << "\r";  // Move cursor to the beginning of the line
    }
}

char getch() {
    char buf = 0;
    if (read(STDIN_FILENO, &buf, 1) < 0) {
        perror("read");
    }
    return buf;
}

void displayOptions(std::string _query, std::vector<std::string> options, int selected) {
    query(_query);

    for (int i = 0; i < options.size(); i++) {
        if (i == selected) {
            std::cout << " \x1b[32m●\x1b[0m " << options[i] << "\n";
        } else {
            std::cout << " \x1b[30m○ " << options[i] << "\x1b[0m\n";
        }
    }
}

int askMulti(std::string query, std::vector<std::string> options) {
    static struct termios oldt, newt;

    std::cout << std::endl;

    // Set raw terminal mode
    int count = options.size();
    int selected = 0;
    bool updated = false;
    bool selecting = true;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    displayOptions(query, options, 0);

    while (selecting) {
        char c = getch();
        if (c == '\033') {
            getch();
            switch (getch()) {
                case KEY_UP:
                    selected = (--selected + count) % count;
                    updated = true;
                    break;
                case KEY_DOWN:
                    selected = (++selected) % count;
                    updated = true;
                    break;
            }
        } else if (c == '\n') {
            selecting = false;
        }

        if (updated) {
            eraseLines(options.size() + 2);
            displayOptions(query, options, selected);
        }
    }

    // Set normal terminal mode
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return selected;
}

std::string ask(std::string _query) {
    std::string answer;
    query(_query + "\x1b[0m", false);
    std::cin >> answer;
    return answer;
}

std::string askAllLangs() {
    // Hashmap of all langs with their ISO code
    const std::map<std::string, std::string> langs = {
        {"Abkhazian", "ab"},
        {"Afar", "aa"},
        {"Afrikaans", "af"},
        {"Akan", "ak"},
        {"Albanian", "sq"},
        {"Amharic", "am"},
        {"Arabic", "ar"},
        {"Aragonese", "an"},
        {"Armenian", "hy"},
        {"Assamese", "as"},
        {"Avaric", "av"},
        {"Avestan", "ae"},
        {"Aymara", "ay"},
        {"Azerbaijani", "az"},
        {"Bambara", "bm"},
        {"Bashkir", "ba"},
        {"Basque", "eu"},
        {"Belarusian", "be"},
        {"Bengali (Bangla)", "bn"},
        {"Bihari", "bh"},
        {"Bislama", "bi"},
        {"Bosnian", "bs"},
        {"Breton", "br"},
        {"Bulgarian", "bg"},
        {"Burmese", "my"},
        {"Catalan", "ca"},
        {"Chamorro", "ch"},
        {"Chechen", "ce"},
        {"Chichewa, Chewa, Nyanja", "ny"},
        {"Chinese", "zh"},
        {"Chinese (Simplified)", "zh-Hans"},
        {"Chinese (Traditional)", "zh-Hant"},
        {"Chuvash", "cv"},
        {"Cornish", "kw"},
        {"Corsican", "co"},
        {"Cree", "cr"},
        {"Croatian", "hr"},
        {"Czech", "cs"},
        {"Danish", "da"},
        {"Divehi, Dhivehi, Maldivian", "dv"},
        {"Dutch", "nl"},
        {"Dzongkha", "dz"},
        {"English", "en"},
        {"Esperanto", "eo"},
        {"Estonian", "et"},
        {"Ewe", "ee"},
        {"Faroese", "fo"},
        {"Fijian", "fj"},
        {"Finnish", "fi"},
        {"French", "fr"},
        {"Fula, Fulah, Pulaar, Pular", "ff"},
        {"Galician", "gl"},
        {"Gaelic (Scottish)", "gd"},
        {"Gaelic (Manx)", "gv"},
        {"Georgian", "ka"},
        {"German", "de"},
        {"Greek", "el"},
        {"Greenlandic", "kl"},
        {"Guarani", "gn"},
        {"Gujarati", "gu"},
        {"Haitian Creole", "ht"},
        {"Hausa", "ha"},
        {"Hebrew", "he"},
        {"Herero", "hz"},
        {"Hindi", "hi"},
        {"Hiri Motu", "ho"},
        {"Hungarian", "hu"},
        {"Icelandic", "is"},
        {"Ido", "io"},
        {"Igbo", "ig"},
        {"Indonesian", "id"},
        {"Interlingua", "ia"},
        {"Interlingue", "ie"},
        {"Inuktitut", "iu"},
        {"Inupiak", "ik"},
        {"Irish", "ga"},
        {"Italian", "it"},
        {"Japanese", "ja"},
        {"Javanese", "jv"},
        {"Kalaallisut, Greenlandic", "kl"},
        {"Kannada", "kn"},
        {"Kanuri", "kr"},
        {"Kashmiri", "ks"},
        {"Kazakh", "kk"},
        {"Khmer", "km"},
        {"Kikuyu", "ki"},
        {"Kinyarwanda (Rwanda)", "rw"},
        {"Kirundi", "rn"},
        {"Kyrgyz", "ky"},
        {"Komi", "kv"},
        {"Kongo", "kg"},
        {"Korean", "ko"},
        {"Kurdish", "ku"},
        {"Kwanyama", "kj"},
        {"Lao", "lo"},
        {"Latin", "la"},
        {"Latvian (Lettish)", "lv"},
        {"Limburgish (Limburger)", "li"},
        {"Lingala", "ln"},
        {"Lithuanian", "lt"},
        {"Luga-Katanga", "lu"},
        {"Luganda, Ganda", "lg"},
        {"Luxembourgish", "lb"},
        {"Manx", "gv"},
        {"Macedonian", "mk"},
        {"Malagasy", "mg"},
        {"Malay", "ms"},
        {"Malayalam", "ml"},
        {"Maltese", "mt"},
        {"Maori", "mi"},
        {"Marathi", "mr"},
        {"Marshallese", "mh"},
        {"Moldavian", "mo"},
        {"Mongolian", "mn"},
        {"Nauru", "na"},
        {"Navajo", "nv"},
        {"Ndonga", "ng"},
        {"Northern Ndebele", "nd"},
        {"Nepali", "ne"},
        {"Norwegian", "no"},
        {"Norwegian bokmål", "nb"},
        {"Norwegian nynorsk", "nn"},
        {"Nuosu", "ii"},
        {"Occitan", "oc"},
        {"Ojibwe", "oj"},
        {"Old Church Slavonic, Old Bulgarian", "cu"},
        {"Oriya", "or"},
        {"Oromo (Afaan Oromo)", "om"},
        {"Ossetian", "os"},
        {"Pāli", "pi"},
        {"Pashto, Pushto", "ps"},
        {"Persian (Farsi)", "fa"},
        {"Polish", "pl"},
        {"Portuguese", "pt"},
        {"Punjabi (Eastern)", "pa"},
        {"Quechua", "qu"},
        {"Romansh", "rm"},
        {"Romanian", "ro"},
        {"Russian", "ru"},
        {"Sami", "se"},
        {"Samoan", "sm"},
        {"Sango", "sg"},
        {"Sanskrit", "sa"},
        {"Serbian", "sr"},
        {"Serbo-Croatian", "sh"},
        {"Sesotho", "st"},
        {"Setswana", "tn"},
        {"Shona", "sn"},
        {"Sichuan Yi", "ii"},
        {"Sindhi", "sd"},
        {"Sinhalese", "si"},
        {"Siswati", "ss"},
        {"Slovak", "sk"},
        {"Slovenian", "sl"},
        {"Somali", "so"},
        {"Southern Ndebele", "nr"},
        {"Spanish", "es"},
        {"Sundanese", "su"},
        {"Swahili (Kiswahili)", "sw"},
        {"Kiswahili", "sw"},
        {"Swati", "ss"},
        {"Swedish", "sv"},
        {"Tagalog", "tl"},
        {"Tahitian", "ty"},
        {"Tajik", "tg"},
        {"Tamil", "ta"},
        {"Tatar", "tt"},
        {"Telugu", "te"},
        {"Thai", "th"},
        {"Tibetan", "bo"},
        {"Tigrinya", "ti"},
        {"Tonga", "to"},
        {"Tsonga", "ts"},
        {"Turkish", "tr"},
        {"Turkmen", "tk"},
        {"Twi", "tw"},
        {"Uyghur", "ug"},
        {"Ukrainian", "uk"},
        {"Urdu", "ur"},
        {"Uzbek", "uz"},
        {"Venda", "ve"},
        {"Vietnamese", "vi"},
        {"Volapük", "vo"},
        {"Wallon", "wa"},
        {"Welsh", "cy"},
        {"Wolof", "wo"},
        {"Western Frisian", "fy"},
        {"Xhosa", "xh"},
        {"Yiddish", "yi"},
        {"Yoruba", "yo"},
        {"Zhuang, Chuang", "za"},
        {"Zulu", "zu"}};

    // Get all keys from langs
    std::vector<std::string> langKeys;
    langKeys.reserve(langs.size());

    for (const auto& pair : langs) {
        langKeys.push_back(pair.first);
    }

    // Formatting the page
    const int langPerPage = 10;
    int numPages = langKeys.size() / langPerPage;
    int index = 0;

    // Navigation
    while (true) {
        std::vector<std::string> page = getSubVector(langKeys, index * langPerPage, (index + 1) * langPerPage);
        if (index != 0)
            page.push_back("← Previous page");

        if (index != numPages)
            page.push_back("Next page →");

        int choosen = askMulti("Choose lang (" + std::to_string(index + 1) + "/" + std::to_string(numPages + 1) + "):", page);

        int pageSize = page.size();
        if (choosen == pageSize - 2) {
            if (index != 0)
                index--;
            else
                index++;

            eraseLines(page.size() + 3);
        } else if (choosen == pageSize - 1) {
            if (index != numPages)
                index++;
            else
                index--;
            eraseLines(page.size() + 3);
        } else {
            return langs.at(page[choosen]);
        }
    }
}
