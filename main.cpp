
// Ver: 1.0.2

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

/**
 * Função auxiliar para remover espaços em branco no início e no final da string
 *
 * @param str string a ser trimada
 * @return string sem espaços no início e no final
 */
string trim(const string& str) {
    // Remove espaços no início e no final
    size_t first = str.find_first_not_of(" \t");

    // Se o início for o final, retorna uma string vazia
    if (first == string::npos) return "";

    // Remove espaços no início
    size_t last = str.find_last_not_of(" \t");

    // Se o final for o início, retorna a string sem espaços
    return str.substr(first, (last - first + 1));
}


/**
 * Função para tratar os links das thumbs.
 * 
 * Essa função verifica se o Link está no formato correto e o ajusta
 * para o formato correto, se necessário.
 * 
 * @param Link string com o link a ser tratado
 * @return string com o link tratado
 */
string tratarLinkThumbs(string Link) {
    
    // Remove espaços no início e no final do Link
    Link.erase(remove(Link.begin(), Link.end(), ' '), Link.end());

    // Verifica se o Link começa com http:// ou https://
    if (Link.substr(0, 7) != "http://" && Link.substr(0, 8) != "https://") {
        // Se o Link não começa com http:// ou https://, exibe uma mensagem de erro
        cout << "Link invlido: " << Link << " (certifique-se de que começa com http:// ou https://)" << endl;
        return "";
    }

    // Verificação do formato do Link
    if (Link.find("/original/") == string::npos) {
        // Se o Link não contiver /original/, tenta ajustar
        if (Link.find("/w227_and_h127_bestv2/") != string::npos) {
            // Se o Link contiver /w227_and_h127_bestv2/, substitui por /original/
            Link.replace(Link.find("/w227_and_h127_bestv2/"), 24, "/original/");
        }
    }

    // Adiciona .jpg apenas se for um Link válido
    if (Link.size() >= 4 && Link.substr(Link.size() - 4) != ".jpg") {
        Link += ".jpg";
    }

    return Link;
}

/**
 * Função auxiliar para extrair o código único de um Link.
 * 
 * Essa função procura por um identificador (geralmente um nome de domínio) no Link
 * e extrai o código único que vem logo após o identificador. Por exemplo, se o
 * Link for "https://example.com/abc123/def456", e o identificador for "example.com/",
 * a função irá retornar "abc123".
 * 
 * @param Link string com o link a ser processado
 * @param identificador string com o identificador a ser procurado
 * @return string com o código único extraído do link
 */
string extractCode(const string& Link, const string& identificador) {
    size_t posStart = Link.find(identificador);
    if (posStart == string::npos) return ""; // Identificador não encontrado

    posStart += identificador.length();
    size_t posEnd = Link.find("/", posStart); // Procurar pelo próximo "/"

    // Se não houver outro "/", considerar o final da string
    return (posEnd == string::npos) ? Link.substr(posStart) : Link.substr(posStart, posEnd - posStart);
}

/**
 * Função para tratar os links dos servidores
 *
 * Essa função trata os links dos servidores para que estejam no formato correto
 * para serem usados na geração do arquivo "lista.txt". Ela remove espaços no
 * início e no final do Link, e extrai o código único da URL. Além disso, ela
 * remove "embed" do início do código único, se presente, e forma o Link no
 * formato final desejado para o arquivo "lista.txt".
 *
 * @param Link string com o link a ser processado
 * @return string com o link tratado
 */
string tratarLinkServidor(string Link) {
    // Remove espaços no início e no final do Link
    Link = trim(Link);

    // Define uma string vazia para o Código Unico
    string CodigoUnico;

    // Formata Links mp4upload.com (/embed- ou /)
    if (Link.find("mp4upload.com") != string::npos) {

        // Tenta capturar o código único = CodigoUnico
        CodigoUnico = extractCode(Link, "embed-");

        // Verificar se o Link foi capturado corretamente
        if (CodigoUnico.empty()) {
            CodigoUnico = extractCode(Link, "mp4upload.com/");
        }

        // Remove "embed" do início do CodigoUnico, se presente
        if (CodigoUnico.rfind("embed", 0) == 0) { // Verifica se "embed" está no início
            CodigoUnico.erase(0, 5); // Remove os 5 caracteres da palavra "embed"
        }

        // Link Final Formatado
        if (!CodigoUnico.empty()) {
            Link = "https://www.mp4upload.com/embed-" + CodigoUnico + ".html"; // O Link final agora deve estar correto
        }

        // Verificar se há múltiplos ".html" no final e remover os excedentes
        while (Link.size() >= 10 && Link.substr(Link.size() - 10) == ".html.html") {
            Link.erase(Link.size() - 5); // Remove o último ".html"
        }

    // Formata Links mixdrop.is ou mixdrop.ps (/e/ ou /f/)
    } else if (Link.find("mixdrop.is") != string::npos || Link.find("mixdrop.ps") != string::npos) {
            // Tenta extrair o código de qualquer uma das variantes
            CodigoUnico = extractCode(Link, "mixdrop.is/e/");
            if (CodigoUnico.empty()) {
                CodigoUnico = extractCode(Link, "mixdrop.is/f/");
            }
            if (CodigoUnico.empty()) {
                CodigoUnico = extractCode(Link, "mixdrop.ps/e/");
            }
            if (!CodigoUnico.empty()) {
                // Sempre retorna no formato "https://mixdrop.ps/e/{codigo}"
                Link = "https://mixdrop.ps/e/" + CodigoUnico;
            }

    // Formata Links streamtape.com (/e/ ou /v/)
    } else if (Link.find("streamtape.com") != string::npos || Link.find("streamtape.to") != string::npos) {
        // Verifica se o Link já está no formato embed "/e/"
        if (Link.find("/e/") == string::npos) {
            // Extrai o código único de "/v/" ou "/e/"
            CodigoUnico = extractCode(Link, "streamtape.com/v/");
            if (CodigoUnico.empty()) {
                CodigoUnico = extractCode(Link, "streamtape.com/e/");
            }
            if (CodigoUnico.empty()) {
                CodigoUnico = extractCode(Link, "streamtape.to/v/");
            }
            if (!CodigoUnico.empty()) {
                Link = "https://streamtape.to/e/" + CodigoUnico;
            }
        }

    // Formata Links filemoon.sx (/e/ ou /d/)        
    } else if (Link.find("filemoon.sx") != string::npos) {
        CodigoUnico = extractCode(Link, "filemoon.sx/e/");
        if (CodigoUnico.empty()) {
            CodigoUnico = extractCode(Link, "filemoon.sx/d/");
            if (CodigoUnico.empty()) {
                CodigoUnico = extractCode(Link, "filemoon.sx/");
            }
        }
        if (!CodigoUnico.empty()) {
            Link = "https://filemoon.sx/e/" + CodigoUnico;
        }

    // Formata Links yourupload.com (/watch/ ou /embed/)
    } else if (Link.find("yourupload.com") != string::npos) {
        // Verifica se o Link contém "/watch/" e extrai o código
        CodigoUnico = extractCode(Link, "/watch/");

        // Se não foi encontrado, tenta extrair do formato embed
        if (CodigoUnico.empty()) {
            CodigoUnico = extractCode(Link, "/embed/");
        }

        // Se o código único foi encontrado, forma o Link no formato embed
        if (!CodigoUnico.empty()) {
            Link = "https://www.yourupload.com/embed/" + CodigoUnico;
        } else {
            cout << "Código único não encontrado no Link: " << Link << endl;
            return ""; // Retorna vazio se nenhum código foi encontrado
        }

    // Formata Links linkbox.to ou lbx.to (/f/ ou /a/f/)
    } else if (Link.find("linkbox.to") != string::npos || Link.find("lbx.to") != string::npos) {
        // Extrai o código único do Link
        CodigoUnico = extractCode(Link, "/f/");

        // Se não encontrou com "f/", tenta extrair com "a/f/"
        if (CodigoUnico.empty()) {
            CodigoUnico = extractCode(Link, "a/f/");
        }

        // Se o código único foi encontrado, forma o Link no formato desejado
        if (!CodigoUnico.empty()) {
            Link = "https://www.linkbox.to/a/f/" + CodigoUnico;
        } else {
            cout << "Código único não encontrado no Link: " << Link << endl;
            return ""; // Retorna vazio se nenhum código foi encontrado
        }
    } else if (Link.find("upstream.to") != string::npos) {
        CodigoUnico = extractCode(Link, "embed-");
        if (CodigoUnico.empty()) {
            CodigoUnico = extractCode(Link, "upstream.to/");
        }

        // Remove "embed" do início do CodigoUnico, se presente
        if (CodigoUnico.rfind("embed", 0) == 0) { // Verifica se "embed" está no início
            CodigoUnico.erase(0, 5); // Remove os 5 caracteres da palavra "embed"
        }

        if (!CodigoUnico.empty()) {
            Link = "https://upstream.to/embed-" + CodigoUnico + ".html"; // O Link final agora deve estar correto
        }
        // Verificar se há múltiplos ".html" no final e remover os excedentes
        while (Link.size() >= 10 && Link.substr(Link.size() - 10) == ".html.html") {
            Link.erase(Link.size() - 5); // Remove o último ".html"
        }
    } else if (Link.find("streamwish.com") != string::npos) {
        // Extrai o código único
        CodigoUnico = extractCode(Link, "streamwish.com/d/");
        if (!CodigoUnico.empty()) {
            Link = "https://playerwish.com/e/" + CodigoUnico;
        }
    } else if (Link.find("playerwish.com") != string::npos) {
        // O Link já está no formato playerwish.com, então só extrai o código para garantir o formato correto
        CodigoUnico = extractCode(Link, "playerwish.com/e/");
        if (!CodigoUnico.empty()) {
            Link = "https://playerwish.com/e/" + CodigoUnico;
        }
    } else {
        cout << "Servidor desconhecido ou formato do Link inválido." << endl;
        return "";
    }

    return Link; // Retorna o Link tratado
}

// Função para capturar os links das thumbs
void capturarThumbs() {
    cout << "Digite os links das Thumbs (precisa informar um Link válido para cada episódio):" << endl;
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
        cout << "Digite os links do servidor " << (i + 1) << " (precisa informar um Link válido para cada episódio):" << endl;
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
            cout << "Erro: qualidade inválida. Aceitas apenas SD, HD ou FHD.\nTente Novamente!" << endl;           
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

    // Aguardar 5 segundos antes de encerrar o programa
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Limpar a tela
    limparTela();

    // Encerrar o programa
    return 0;
}
