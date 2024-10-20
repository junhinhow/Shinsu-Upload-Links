
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm> // Para a fun��o remove
#include <clocale>   // Para setlocale
#include <regex>     // Para express�es regulares
#include <thread>    // Para std::this_thread::sleep_for
#include <chrono>    // Para std::chrono::seconds
#include <cstdlib>   // Para system

using namespace std;

// Vari�veis globais
int quantidadeEpisodios, quantidadeServidores, episodioInicial;
vector<string> thumbs;
vector<vector<string>> servidores;
string qualidade; // Agora � uma �nica qualidade para todos os epis�dios

// Fun��o auxiliar para remover espa�os em branco no in�cio e no final da string
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}

// Fun��o para tratar os links das thumbs
string tratarLinkThumbs(string link) {
    link.erase(remove(link.begin(), link.end(), ' '), link.end());

    if (link.substr(0, 7) != "http://" && link.substr(0, 8) != "https://") {
        cout << "Link inv�lido: " << link << " (certifique-se de que come�a com http:// ou https://)" << endl;
        return "";
    }

    // Verifica��o do formato do link
    if (link.find("/original/") == string::npos) {
        if (link.find("/w227_and_h127_bestv2/") != string::npos) {
            link.replace(link.find("/w227_and_h127_bestv2/"), 24, "/original/");
        }
    }

    // Adiciona .jpg apenas se for um link v�lido
    if (link.size() >= 4 && link.substr(link.size() - 4) != ".jpg") {
        link += ".jpg";
    }

    return link;
}

// Fun��o para remover h�fens e colchetes de qualidade do link
string removerHifenEColchetes(string link) {
    link.erase(remove(link.begin(), link.end(), '-'), link.end());

    size_t startPos = link.find("[");
    while (startPos != string::npos) {
        size_t endPos = link.find("]", startPos);
        if (endPos != string::npos) {
            link.erase(startPos, endPos - startPos + 1);
        } else {
            link.erase(startPos);
            break;
        }
        startPos = link.find("[", startPos);
    }

    link.erase(remove(link.begin(), link.end(), '['), link.end());
    link.erase(remove(link.begin(), link.end(), ']'), link.end());

    const string qualidades[] = {"SD", "HD", "FHD"};
    for (const auto& qualidade : qualidades) {
        size_t pos;
        while ((pos = link.find(qualidade)) != string::npos) {
            link.erase(pos, qualidade.length());
        }
    }

    link.erase(remove_if(link.begin(), link.end(), ::isspace), link.end());

    return link;
}

// Fun��o auxiliar para extrair o c�digo �nico de um link
string extractCode(const string& link, const string& identifier) {
    size_t posStart = link.find(identifier);
    if (posStart == string::npos) return ""; // Identificador n�o encontrado

    posStart += identifier.length();
    size_t posEnd = link.find("/", posStart); // Procurar pelo pr�ximo "/"

    // Se n�o houver outro "/", considerar o final da string
    return (posEnd == string::npos) ? link.substr(posStart) : link.substr(posStart, posEnd - posStart);
}

// Fun��o para tratar os links dos servidores
string tratarLinkServidor(string link) {
    // Remove espa�os no in�cio e no final do link
    link = trim(link);

    // Remove h�fens e colchetes
    link = removerHifenEColchetes(link);

    string uniqueCode;

    // Verificar qual servidor est� sendo utilizado pelo link
    if (link.find("mp4upload.com") != string::npos) {
        uniqueCode = extractCode(link, "embed-");
        if (uniqueCode.empty()) {
            uniqueCode = extractCode(link, "mp4upload.com/");
        }

        // Remove "embed" do in�cio do uniqueCode, se presente
        if (uniqueCode.rfind("embed", 0) == 0) { // Verifica se "embed" est� no in�cio
            uniqueCode.erase(0, 5); // Remove os 5 caracteres da palavra "embed"
        }

        if (!uniqueCode.empty()) {
            link = "https://www.mp4upload.com/embed-" + uniqueCode + ".html"; // O link final agora deve estar correto
        }

        // Verificar se h� m�ltiplos ".html" no final e remover os excedentes
        while (link.size() >= 10 && link.substr(link.size() - 10) == ".html.html") {
            link.erase(link.size() - 5); // Remove o �ltimo ".html"
        }
    } else if (link.find("mixdrop.is") != string::npos || link.find("mixdrop.ps") != string::npos) {
            // Tenta extrair o c�digo de qualquer uma das variantes
            uniqueCode = extractCode(link, "mixdrop.is/e/");
            if (uniqueCode.empty()) {
                uniqueCode = extractCode(link, "mixdrop.is/f/");
            }
            if (uniqueCode.empty()) {
                uniqueCode = extractCode(link, "mixdrop.ps/e/");
            }
            if (!uniqueCode.empty()) {
                // Sempre retorna no formato "https://mixdrop.ps/e/{codigo}"
                link = "https://mixdrop.ps/e/" + uniqueCode;
            }
    } else if (link.find("streamtape.com") != string::npos || link.find("streamtape.to") != string::npos) {
        // Verifica se o link j� est� no formato embed "/e/"
        if (link.find("/e/") == string::npos) {
            // Extrai o c�digo �nico de "/v/" ou "/e/"
            uniqueCode = extractCode(link, "streamtape.com/v/");
            if (uniqueCode.empty()) {
                uniqueCode = extractCode(link, "streamtape.com/e/");
            }
            if (uniqueCode.empty()) {
                uniqueCode = extractCode(link, "streamtape.to/v/");
            }
            if (!uniqueCode.empty()) {
                link = "https://streamtape.to/e/" + uniqueCode;
            }
        }
    } else if (link.find("filemoon.sx") != string::npos) {
        uniqueCode = extractCode(link, "filemoon.sx/e/");
        if (uniqueCode.empty()) {
            uniqueCode = extractCode(link, "filemoon.sx/d/");
            if (uniqueCode.empty()) {
                uniqueCode = extractCode(link, "filemoon.sx/");
            }
        }
        if (!uniqueCode.empty()) {
            link = "https://filemoon.sx/e/" + uniqueCode;
        }
    } else if (link.find("yourupload.com") != string::npos) {
        // Verifica se o link cont�m "/watch/" e extrai o c�digo
        uniqueCode = extractCode(link, "/watch/");

        // Se n�o foi encontrado, tenta extrair do formato embed
        if (uniqueCode.empty()) {
            uniqueCode = extractCode(link, "/embed/");
        }

        // Se o c�digo �nico foi encontrado, forma o link no formato embed
        if (!uniqueCode.empty()) {
            link = "https://www.yourupload.com/embed/" + uniqueCode;
        } else {
            cout << "C�digo �nico n�o encontrado no link: " << link << endl;
            return ""; // Retorna vazio se nenhum c�digo foi encontrado
        }
    } else if (link.find("linkbox.to") != string::npos || link.find("lbx.to") != string::npos) {
        // Extrai o c�digo �nico do link
        uniqueCode = extractCode(link, "/f/");

        // Se n�o encontrou com "f/", tenta extrair com "a/f/"
        if (uniqueCode.empty()) {
            uniqueCode = extractCode(link, "a/f/");
        }

        // Se o c�digo �nico foi encontrado, forma o link no formato desejado
        if (!uniqueCode.empty()) {
            link = "https://www.linkbox.to/a/f/" + uniqueCode;
        } else {
            cout << "C�digo �nico n�o encontrado no link: " << link << endl;
            return ""; // Retorna vazio se nenhum c�digo foi encontrado
        }
    } else if (link.find("upstream.to") != string::npos) {
        uniqueCode = extractCode(link, "embed-");
        if (uniqueCode.empty()) {
            uniqueCode = extractCode(link, "upstream.to/");
        }

        // Remove "embed" do in�cio do uniqueCode, se presente
        if (uniqueCode.rfind("embed", 0) == 0) { // Verifica se "embed" est� no in�cio
            uniqueCode.erase(0, 5); // Remove os 5 caracteres da palavra "embed"
        }

        if (!uniqueCode.empty()) {
            link = "https://upstream.to/embed-" + uniqueCode + ".html"; // O link final agora deve estar correto
        }
        // Verificar se h� m�ltiplos ".html" no final e remover os excedentes
        while (link.size() >= 10 && link.substr(link.size() - 10) == ".html.html") {
            link.erase(link.size() - 5); // Remove o �ltimo ".html"
        }
    } else if (link.find("streamwish.com") != string::npos) {
        // Extrai o c�digo �nico
        uniqueCode = extractCode(link, "streamwish.com/d/");
        if (!uniqueCode.empty()) {
            link = "https://playerwish.com/e/" + uniqueCode;
        }
    } else if (link.find("playerwish.com") != string::npos) {
        // O link j� est� no formato playerwish.com, ent�o s� extrai o c�digo para garantir o formato correto
        uniqueCode = extractCode(link, "playerwish.com/e/");
        if (!uniqueCode.empty()) {
            link = "https://playerwish.com/e/" + uniqueCode;
        }
    } else {
        cout << "Servidor desconhecido ou formato do link inv�lido." << endl;
        return "";
    }

    return link; // Retorna o link tratado
}

// Fun��o para capturar os links das thumbs
void capturarThumbs() {
    cout << "Digite os links das Thumbs (precisa informar um link v�lido para cada epis�dio):" << endl;
    cin.ignore(); // Ignora qualquer caractere de nova linha deixado no buffer pelo cin anterior

    int capturados = 0;

    while (capturados < quantidadeEpisodios) {
        string thumb;
        getline(cin, thumb);

        if (thumb == "0") {
            cout << "Voc� precisa capturar " << quantidadeEpisodios << " links. Ainda faltam " << (quantidadeEpisodios - capturados) << " links." << endl;
            continue;
        }

        string linkTratado = tratarLinkThumbs(thumb);
        if (!linkTratado.empty()) {
            thumbs.push_back(linkTratado);
            capturados++;
        } else {
            cout << "Link inv�lido. Por favor, tente novamente." << endl;
        }
    }

    cout << "Todos os links de thumbs capturados com sucesso!" << endl;
}

// Fun��o para capturar os links dos servidores
void capturarServidores() {
    servidores.resize(quantidadeServidores);

    for (int i = 0; i < quantidadeServidores; ++i) {
        cout << "Digite os links do servidor " << (i + 1) << " (precisa informar um link v�lido para cada epis�dio):" << endl;
        int capturados = 0;

        while (capturados < quantidadeEpisodios) {
            string servidor;
            getline(cin, servidor);

            if (servidor == "0") {
                cout << "Voc� precisa capturar " << quantidadeEpisodios << " links. Ainda faltam " << (quantidadeEpisodios - capturados) << " links para o servidor " << (i + 1) << "." << endl;
                continue;
            }

            string linkTratado = tratarLinkServidor(servidor);
            if (linkTratado != "") {
                servidores[i].push_back(linkTratado);
                capturados++;
            } else {
                cout << "Link inv�lido. Por favor, tente novamente." << endl;
            }
        }

        cout << "Todos os links do servidor " << (i + 1) << " capturados com sucesso!" << endl;
    }
}

// Fun��o para limpar a tela
void limparTela() {
    system("clear || cls"); // 'clear' para Linux e 'cls' para Windows
}

// Fun��o para solicitar a qualidade dos epis�dios
void solicitarQualidade() {
    while (true) {
        cout << "Digite a qualidade dos epis�dios (SD/HD/FHD): ";
        string input;
        cin >> input;

        // Converter para mai�sculas
        for (char &c : input) {
            c = toupper(c);
        }

        // Verificar se a qualidade est� entre as op��es v�lidas
        if (input == "SD" || input == "HD" || input == "FHD") {
            qualidade = input; // Armazena a qualidade em mai�sculas
            break; // Sai do loop se a entrada for v�lida
        } else {
            cout << "Erro: qualidade inv�lida. Aceitas apenas SD, HD ou FHD." << endl;
            this_thread::sleep_for(chrono::seconds(3)); // Aguarda 3 segundos
            limparTela(); // Limpa a tela
        }
    }
}

// Fun��o para gerar o arquivo "lista.txt"
void gerarArquivo() {
    ofstream arquivo("lista.txt");
    if (arquivo.is_open()) {
        for (int i = episodioInicial; i < episodioInicial + quantidadeEpisodios; i++) {
            arquivo << "id: " << (i) << endl;
            for (int j = 0; j < quantidadeServidores; ++j) {
                if (j < servidores.size() && (i - episodioInicial) < servidores[j].size()) {
                    arquivo << "- " << servidores[j][i-episodioInicial] << " [" << qualidade << "]" << endl;
                }
            }
            if ((i-episodioInicial) < thumbs.size()) {
                arquivo << "thumb: " << thumbs[i-episodioInicial] << endl;
            }

            // Adiciona "----------" apenas se n�o for o �ltimo ID
            if ((i-episodioInicial) < quantidadeEpisodios - 1) {
                arquivo << "----------" << endl;
            }
        }
        arquivo.close();
        cout << "Arquivo 'lista.txt' gerado com sucesso!" << endl;
    } else {
        cout << "N�o foi poss�vel abrir o arquivo para escrita." << endl;
    }
}


// Fun��o para mostrar o conte�do do arquivo gerado
void mostrarArquivoGerado(const string& nomeArquivo) {
    ifstream arquivo(nomeArquivo);
    if (arquivo.is_open()) {
        string linha;
        while (getline(arquivo, linha)) {
            cout << linha << endl; // Exibe cada linha do arquivo
        }
        arquivo.close();
    } else {
        cout << "N�o foi poss�vel abrir o arquivo: " << nomeArquivo << endl;
    }
}

int main() {
    // Define a localiza��o para UTF-8
    setlocale(LC_ALL, "pt_BR.UTF-8");

    // Solicitar quantidade de epis�dios
    cout << "Digite a quantidade de epis�dios: ";
    cin >> quantidadeEpisodios;
    cin.ignore(); // Ignora o newline deixado pelo cin

    cout << "Digite o n�mero do epis�dio inicial: ";
    cin >> episodioInicial;
    cin.ignore(); // Ignora o newline deixado pelo cin

    // Solicitar quantidade de servidores
    cout << "Digite a quantidade de servidores: ";
    cin >> quantidadeServidores;
    cin.ignore(); // Ignora o newline deixado pelo cin

    // Chamar fun��es
    solicitarQualidade();
    capturarThumbs();
    capturarServidores();
    gerarArquivo();
    limparTela();
    mostrarArquivoGerado("lista.txt");


    return 0;
}
