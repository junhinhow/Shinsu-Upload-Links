
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm> // Para a função remove
#include <clocale>   // Para setlocale
#include <regex>     // Para expressões regulares
#include <thread>    // Para std::this_thread::sleep_for
#include <chrono>    // Para std::chrono::seconds
#include <cstdlib>   // Para system

using namespace std;

// Variáveis globais
int quantidadeEpisodios, quantidadeServidores, episodioInicial;
vector<string> thumbs;
vector<vector<string>> servidores;
string qualidade; // Uma única qualidade para todos os episódios

// Funço auxiliar para remover espaços em branco no início e no final da string
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}

// Função para tratar os links das thumbs
string tratarLinkThumbs(string link) {
    link.erase(remove(link.begin(), link.end(), ' '), link.end());

    if (link.substr(0, 7) != "http://" && link.substr(0, 8) != "https://") {
        cout << "Link invlido: " << link << " (certifique-se de que começa com http:// ou https://)" << endl;
        return "";
    }

    // Verificação do formato do link
    if (link.find("/original/") == string::npos) {
        if (link.find("/w227_and_h127_bestv2/") != string::npos) {
            link.replace(link.find("/w227_and_h127_bestv2/"), 24, "/original/");
        }
    }

    // Adiciona .jpg apenas se for um link válido
    if (link.size() >= 4 && link.substr(link.size() - 4) != ".jpg") {
        link += ".jpg";
    }

    return link;
}

// Função para remover hífens e colchetes de qualidade do link
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

// Função auxiliar para extrair o código único de um link
string extractCode(const string& link, const string& identifier) {
    size_t posStart = link.find(identifier);
    if (posStart == string::npos) return ""; // Identificador não encontrado

    posStart += identifier.length();
    size_t posEnd = link.find("/", posStart); // Procurar pelo próximo "/"

    // Se não houver outro "/", considerar o final da string
    return (posEnd == string::npos) ? link.substr(posStart) : link.substr(posStart, posEnd - posStart);
}

// Função para tratar os links dos servidores
string tratarLinkServidor(string link) {
    // Remove espaços no início e no final do link
    link = trim(link);

    // Remove hífens e colchetes
    link = removerHifenEColchetes(link);

    string uniqueCode;

    // Verificar qual servidor está sendo utilizado pelo link
    if (link.find("mp4upload.com") != string::npos) {
        uniqueCode = extractCode(link, "embed-");
        if (uniqueCode.empty()) {
            uniqueCode = extractCode(link, "mp4upload.com/");
        }

        // Remove "embed" do início do uniqueCode, se presente
        if (uniqueCode.rfind("embed", 0) == 0) { // Verifica se "embed" está no início
            uniqueCode.erase(0, 5); // Remove os 5 caracteres da palavra "embed"
        }

        if (!uniqueCode.empty()) {
            link = "https://www.mp4upload.com/embed-" + uniqueCode + ".html"; // O link final agora deve estar correto
        }

        // Verificar se há múltiplos ".html" no final e remover os excedentes
        while (link.size() >= 10 && link.substr(link.size() - 10) == ".html.html") {
            link.erase(link.size() - 5); // Remove o último ".html"
        }
    } else if (link.find("mixdrop.is") != string::npos || link.find("mixdrop.ps") != string::npos) {
            // Tenta extrair o código de qualquer uma das variantes
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
        // Verifica se o link já está no formato embed "/e/"
        if (link.find("/e/") == string::npos) {
            // Extrai o código único de "/v/" ou "/e/"
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
        // Verifica se o link contém "/watch/" e extrai o código
        uniqueCode = extractCode(link, "/watch/");

        // Se não foi encontrado, tenta extrair do formato embed
        if (uniqueCode.empty()) {
            uniqueCode = extractCode(link, "/embed/");
        }

        // Se o código único foi encontrado, forma o link no formato embed
        if (!uniqueCode.empty()) {
            link = "https://www.yourupload.com/embed/" + uniqueCode;
        } else {
            cout << "Código único não encontrado no link: " << link << endl;
            return ""; // Retorna vazio se nenhum código foi encontrado
        }
    } else if (link.find("linkbox.to") != string::npos || link.find("lbx.to") != string::npos) {
        // Extrai o código único do link
        uniqueCode = extractCode(link, "/f/");

        // Se não encontrou com "f/", tenta extrair com "a/f/"
        if (uniqueCode.empty()) {
            uniqueCode = extractCode(link, "a/f/");
        }

        // Se o código único foi encontrado, forma o link no formato desejado
        if (!uniqueCode.empty()) {
            link = "https://www.linkbox.to/a/f/" + uniqueCode;
        } else {
            cout << "Código único não encontrado no link: " << link << endl;
            return ""; // Retorna vazio se nenhum código foi encontrado
        }
    } else if (link.find("upstream.to") != string::npos) {
        uniqueCode = extractCode(link, "embed-");
        if (uniqueCode.empty()) {
            uniqueCode = extractCode(link, "upstream.to/");
        }

        // Remove "embed" do início do uniqueCode, se presente
        if (uniqueCode.rfind("embed", 0) == 0) { // Verifica se "embed" está no início
            uniqueCode.erase(0, 5); // Remove os 5 caracteres da palavra "embed"
        }

        if (!uniqueCode.empty()) {
            link = "https://upstream.to/embed-" + uniqueCode + ".html"; // O link final agora deve estar correto
        }
        // Verificar se há múltiplos ".html" no final e remover os excedentes
        while (link.size() >= 10 && link.substr(link.size() - 10) == ".html.html") {
            link.erase(link.size() - 5); // Remove o último ".html"
        }
    } else if (link.find("streamwish.com") != string::npos) {
        // Extrai o código único
        uniqueCode = extractCode(link, "streamwish.com/d/");
        if (!uniqueCode.empty()) {
            link = "https://playerwish.com/e/" + uniqueCode;
        }
    } else if (link.find("playerwish.com") != string::npos) {
        // O link já está no formato playerwish.com, então só extrai o código para garantir o formato correto
        uniqueCode = extractCode(link, "playerwish.com/e/");
        if (!uniqueCode.empty()) {
            link = "https://playerwish.com/e/" + uniqueCode;
        }
    } else {
        cout << "Servidor desconhecido ou formato do link inválido." << endl;
        return "";
    }

    return link; // Retorna o link tratado
}

// Função para capturar os links das thumbs
void capturarThumbs() {
    cout << "Digite os links das Thumbs (precisa informar um link válido para cada episódio):" << endl;
    cin.ignore(); // Ignora qualquer caractere de nova linha deixado no buffer pelo cin anterior

    int capturados = 0;

    while (capturados < quantidadeEpisodios) {
        string thumb;
        getline(cin, thumb);

        if (thumb == "0") {
            cout << "Você precisa capturar " << quantidadeEpisodios << " links. Ainda faltam " << (quantidadeEpisodios - capturados) << " links." << endl;
            continue;
        }

        string linkTratado = tratarLinkThumbs(thumb);
        if (!linkTratado.empty()) {
            thumbs.push_back(linkTratado);
            capturados++;
        } else {
            cout << "Link inválido. Por favor, tente novamente." << endl;
        }
    }

    cout << "Todos os links de thumbs capturados com sucesso!" << endl;
}

// Função para capturar os links dos servidores
void capturarServidores() {
    servidores.resize(quantidadeServidores);

    for (int i = 0; i < quantidadeServidores; ++i) {
        cout << "Digite os links do servidor " << (i + 1) << " (precisa informar um link válido para cada episódio):" << endl;
        int capturados = 0;

        while (capturados < quantidadeEpisodios) {
            string servidor;
            getline(cin, servidor);

            if (servidor == "0") {
                cout << "Você precisa capturar " << quantidadeEpisodios << " links. Ainda faltam " << (quantidadeEpisodios - capturados) << " links para o servidor " << (i + 1) << "." << endl;
                continue;
            }

            string linkTratado = tratarLinkServidor(servidor);
            if (linkTratado != "") {
                servidores[i].push_back(linkTratado);
                capturados++;
            } else {
                cout << "Link inválido. Por favor, tente novamente." << endl;
            }
        }

        cout << "Todos os links do servidor " << (i + 1) << " capturados com sucesso!" << endl;
    }
}

// Função para limpar a tela
void limparTela() {
    system("clear || cls"); // 'clear' para Linux e 'cls' para Windows
}

// Função para solicitar a qualidade dos episódios
void solicitarQualidade() {
    while (true) {
        cout << "Digite a qualidade dos episódios (SD/HD/FHD): ";
        string input;
        cin >> input;

        // Converter para maiúsculas
        for (char &c : input) {
            c = toupper(c);
        }

        // Verificar se a qualidade está entre as opções válidas
        if (input == "SD" || input == "HD" || input == "FHD") {
            qualidade = input; // Armazena a qualidade em maiúsculas
            break; // Sai do loop se a entrada for válida
        } else {
            cout << "Erro: qualidade inválida. Aceitas apenas SD, HD ou FHD." << endl;
            this_thread::sleep_for(chrono::seconds(3)); // Aguarda 3 segundos
            limparTela(); // Limpa a tela
        }
    }
}

// Função para gerar o arquivo "lista.txt"
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

            // Adiciona "----------" apenas se não for o último ID
            if ((i-episodioInicial) < quantidadeEpisodios - 1) {
                arquivo << "----------" << endl;
            }
        }
        arquivo.close();
        cout << "Arquivo 'lista.txt' gerado com sucesso!" << endl;
    } else {
        cout << "não foi possível abrir o arquivo para escrita." << endl;
    }
}


// Função para mostrar o conteúdo do arquivo gerado
void mostrarArquivoGerado(const string& nomeArquivo) {
    ifstream arquivo(nomeArquivo);
    if (arquivo.is_open()) {
        string linha;
        while (getline(arquivo, linha)) {
            cout << linha << endl; // Exibe cada linha do arquivo
        }
        arquivo.close();
    } else {
        cout << "Não foi possível abrir o arquivo: " << nomeArquivo << endl;
    }
}

int main() {
    // Define a localização para UTF-8
    setlocale(LC_ALL, "pt_BR.UTF-8");

    // Solicitar quantidade de episódios
    cout << "Digite a quantidade de episódios: ";
    cin >> quantidadeEpisodios;
    cin.ignore(); // Ignora o newline deixado pelo cin

    cout << "Digite o número do episódio inicial: ";
    cin >> episodioInicial;
    cin.ignore(); // Ignora o newline deixado pelo cin

    // Solicitar quantidade de servidores
    cout << "Digite a quantidade de servidores: ";
    cin >> quantidadeServidores;
    cin.ignore(); // Ignora o newline deixado pelo cin

    // Chamar funções
    solicitarQualidade();
    capturarThumbs();
    capturarServidores();
    gerarArquivo();
    limparTela();
    mostrarArquivoGerado("lista.txt");


    return 0;
}
