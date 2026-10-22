# Instro Natural 0.3 — instrumentos gravados

Instrumento para Windows 64 bits, com plugin VST3 e aplicativo independente. A biblioteca local contém 7.350 arquivos WAV, aproximadamente 7,2 GB de áudio, organizados em 57 opções de instrumentos e combinações, com 248 articulações/presets. Os 80 timbres sintéticos da versão anterior também estão disponíveis.

O catálogo inclui piano, violões de nylon e aço, guitarras elétricas, baixo elétrico, violino solo, conjuntos de violinos/violas/cellos, contrabaixo acústico, harpas, flautas, clarinete, oboé, fagote, sax tenor, trompete, trombone, trompa, tuba, órgãos, cravos, kalimbas, vibrafone, marimba, xilofone, glockenspiel e várias percussões. Veja CATALOGO.md para a lista completa. As 57 opções incluem guitarras combinadas e pianos alternativos; articulações não são contadas como instrumentos separados.

## Experimentar sem Ableton

1. Abra `Standalone/Instro Natural.exe`.
2. Configure a saída de áudio no painel do aplicativo, se necessário.
3. Se a biblioteca não for localizada automaticamente, clique em **Escolher pasta Banks...** e selecione a pasta `Banks` deste pacote.
4. Escolha família, instrumento e articulação. Aguarde **Pronto** e toque no teclado da interface ou em seu controlador MIDI.

O arquivo `Demo-instrumentos.wav` permite ouvir dez exemplos. A sequência está descrita em `Validation/demo-index.json`.

## Usar no Ableton Live 12

1. Copie a pasta completa `VST3/Instro Natural.vst3` para `C:\Program Files\Common Files\VST3`. O Windows pode solicitar permissão de administrador para essa cópia. Outra opção é usar a pasta `VST3` deste pacote como pasta personalizada de VST3 no Live.
2. Nas configurações de Plug-ins do Live, habilite a pasta correspondente e faça uma nova busca (**Rescan**).
3. Adicione **Instro Natural** a uma faixa MIDI.
4. Na interface do plugin, clique em **Escolher pasta Banks...**, selecione a pasta `Banks` fornecida e escolha o instrumento.
5. Espere **Pronto**, arme a faixa e envie notas MIDI.

Mantenha `Banks` em um local fixo, preferencialmente em SSD. Os caminhos do banco e do preset são salvos no projeto do Live. Se mover a biblioteca, selecione novamente a pasta Banks e o instrumento e salve o projeto. O carregamento ocorre em segundo plano; aguarde a indicação Pronto após abrir o projeto. Os samples não ficam incorporados no projeto do Live.

Referência de instalação do fabricante: https://help.ableton.com/hc/en-us/articles/209071729-Using-VST-plug-ins-on-Windows

## Tocar e variar

- **Sons naturais** ativado: reproduz gravações reais. **OUTPUT** controla o volume. O seletor de articulação escolhe técnicas gravadas quando disponíveis.
- A intensidade MIDI (**velocity**) seleciona camadas e/ou controla o volume, conforme o instrumento. Nem todos os bancos têm a mesma quantidade de camadas.
- Sustain (**CC64**), pitch bend e aftertouch são encaminhados ao motor. A resposta depende do SFZ. **CC1/CC11** só modificam o som quando o mapeamento usa esses controles.
- Os presets marcados **Articulacoes por keyswitch** usam notas de troca do próprio banco. Para uso mais simples, prefira as articulações individuais do menu.
- O violão de nylon tem modos automático, dedos e palheta. O baixo Growlybass oferece sete mapeamentos, incluindo notas limpas e staccato. As guitarras incluem twang, staccato e outras técnicas.
- **Parar notas** interrompe o som natural. Ao trocar de instrumento, as notas anteriores são encerradas.
- Desative **Sons naturais** para acessar os controles do sintetizador e seus 80 timbres. A mudança de modo encerra as notas em execução.

## O que foi integrado

Bibliotecas: Meta Open Sound (piano e dois violões), VSCO 2 Community Edition (orquestra), seleção de 26 instrumentos VCSL, Black And Green Guitars e Growlybass da Karoryfer. Os bancos selecionados usam CC0; os textos originais de licença estão junto dos arquivos. Referências e hashes dos downloads constam em `download-provenance.json`.

Motor: sfizz 1.2.3, integrado ao projeto JUCE 9.0.4. O carregamento de arquivos usa uma thread de trabalho. O plugin toca um instrumento SFZ por instância, em mono ou estéreo, com até 128 vozes no modo natural. Para tocar vários instrumentos ao mesmo tempo, use várias instâncias/faixas MIDI. Os canais MIDI de entrada alimentam o mesmo instrumento; não há multitimbralidade ou MPE.

## Limites desta versão

- Um teste de carregar e produzir áudio confirma integridade básica, mas não substitui avaliação musical de toda a extensão de cada instrumento. Algumas gravações têm ruído, afinação ou dinâmica menos uniforme que bibliotecas comerciais.
- Os mapas VCSL foram criados para este projeto a partir de nomes de notas, intensidades e repetições. Samples de soltura não foram associados, e não foram criados loops artificiais de sustentação. Gravações longas terminam ao chegar ao fim do áudio.
- As guitarras Black And Green usam algumas modulações SFZ não implementadas no sfizz 1.2.3: `ampeg_attack_shape` e determinadas modulações entre LFOs. As gravações e articulações principais produzem som, mas esses detalhes podem diferir do Sforzando. `group_label` não afeta o áudio.
- A troca de preset natural é feita na interface e salva no estado do projeto; ainda não é um parâmetro de automação do host. Durante o carregamento, o instrumento anterior permanece disponível até a troca.
- Se houver perda de arquivos ou falha de carga, a interface informa o erro e mantém o último instrumento carregado. Um projeto aberto em outro computador precisa da biblioteca nos caminhos correspondentes ou de nova seleção manual.
- A saída possui proteção por limitação em ±1. Reduza OUTPUT se notas/acordes fortes soarem saturados.
- A confirmação de uso no Ableton exige abrir e tocar no seu Live. Consulte `Validation` para os resultados técnicos efetivamente executados.

## Código e licenças

O código está em `Source`, com as dependências JUCE e sfizz em arquivos de origem para recompilação. A versão aberta do código Instro usa AGPL-3.0-or-later. Essa licença do programa é independente da licença CC0 dos bancos. Os avisos das dependências estão em `Licenses`.

O projeto original em Downloads não foi alterado. Instro Natural usa identificação própria de plugin e pode coexistir com o Instro anterior.
