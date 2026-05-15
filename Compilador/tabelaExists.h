bool exists(SymbolTable[] tabela, char[]* identificador){
    int i, j, maiorNivel;
    i, j = 0;
    
    int tamanhoTabela;
    tamanhoTabela = sizeof(tabela) / sizeof(tabela[0])
    
    maiorNivel = tabela[tamanhoTabela - 1].escopo
    
    while(i <= maiorNivel || j < tamanhoTabela){
        if(tabela[j].escopo <= maiorNivel && strcmp(tabela[j].identificador, identificador)){
            return true
        }
        j++
        i = tabela[j].escopo
    }
    return false
}