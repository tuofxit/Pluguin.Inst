# Validacao realizada

- 248/248 presets SFZ carregaram e produziram audio finito, em duas intensidades (40 e 110).
- 1.081 hashes do pacote Meta conferidos com SHA256SUMS dos autores (inclui arquivos de mapeamento).
- 951 samples VCSL conferidos com hashes Git durante o download.
- 20.099 referencias de samples conferidas por nome de arquivo e 240 includes por caminho relativo; nenhuma ausencia encontrada. A checagem de nomes nao substitui teste de todas as notas.
- O processador compilado produziu audio em 44,1/48 kHz com blocos de 64/1024/4096 frames. Restauracao de estado, recuperacao apos arquivo ausente e 80 timbres sinteticos passaram.
- A interface foi renderizada em Interface.png e inspecionada visualmente.
- pluginval 1.0.4: nivel 5, SUCCESS. Testes de GUI desativados nessa ferramenta; renderizacao da interface testada separadamente. O validador externo Steinberg nao foi executado.
- Nao foi executada uma sessao no Ableton Live 12. O teste final no host ainda cabe ao usuario.

Os logs e resultados detalhados estao nesta pasta. A demonstracao usa o sfizz 1.2.3 oficial e os mesmos bancos, em renderizacao offline, sem efeitos externos.
