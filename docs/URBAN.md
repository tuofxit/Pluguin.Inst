# Expansão Urban 0.4.0

149 presets novos tocáveis por MIDI: 95 one shots, 14 kits de bateria/efeitos, 8 teclas/texturas SK5, 8 timbres MS20, 12 pads/leads SHLD e 12 instrumentos originais (8 baixos 808 e 4 bells/plucks). Com os bancos anteriores: 397 presets de samples, além dos 80 sintetizadores do plugin.

## Baixar e usar no Ableton Live 12

Baixe Baixar-Instro.ps1 na release 0.4.0 e execute no PowerShell:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Baixar-Instro.ps1 -Bancos urban
```

Para todos os instrumentos, use `-Bancos todos`. O banco Urban é independente dos bancos acústicos. Quem já tem a 0.3 pode usar a mesma pasta de destino: o catálogo e o programa são atualizados, os bancos anteriores continuam no lugar. O download Urban contém os samples; o ZIP Windows sozinho contém apenas o programa, catálogo e documentação.

1. Instale/copie o pacote `Instro Natural.vst3` na pasta VST3 do Windows ou configure a pasta VST3 personalizada no Live.
2. Arraste Instro Natural para uma faixa MIDI. Ative **Usar samples**, escolha a pasta **Banks** e aguarde **Pronto**.
3. Escolha família, instrumento e variação. Crie um clip MIDI e desenhe as notas ou toque no teclado.
4. Use uma instância por instrumento/faixa: 808 em uma faixa, pad em outra, bateria em outra.

## Sons para começar

| Produção | Opções sugeridas |
|---|---|
| Trap | 808 Clean/Hard/Long, MS20 Acid Bass, TX81Z hats/snares, Bell Glass |
| Emo trap | Greenpad, MiniFreak, Pluck Soft; combine com os violões e guitarras dos bancos anteriores |
| Lo-fi | SK5 Piano/Vibraphone/PipeOrgan, 808 Soft, SR16 Jazz, Bell Dark |
| Funk | 808 Short/Saturated, MS20 drums, TX81Z agogôs/timbales e one shots; base para programar seu ritmo |
| Pop | Pro1 Biglead, Greenpad, SK5, SR16 Rock |
| Eletrônica | MS20 Acid/Reso Bass, ondas Saw/Square, Wavestate, lasers e efeitos |

Essas são sugestões de uso, não packs de ritmos autênticos de cada gênero. Os 808s originais são sintetizados e renderizados em WAV, não gravações de uma TR-808. Os dois Biglead gratuitos vêm identificados como LQ pelo autor. Não foi incluído um pack específico de funk brasileiro.

## Notas e kits

Kits Frequency303 usam MIDI 36 em diante: consulte MAPA-TECLAS.md. One shots individuais mantêm sua afinação original na nota MIDI 60 e podem ser transpostos. Kits SR16 preservam os números originais de notas do autor (36–51); use o piano roll para localizar os sons. Os WAV de loops SK5 podem ser arrastados para uma faixa de áudio e ajustados com Warp no Live. O Instro não sincroniza esses loops automaticamente ao andamento.

MS20/SK5 respeitam as notas raiz e os loops embutidos nos WAV; o nome de oitava de DirectWave difere do Live, portanto o mapeamento usa os metadados MIDI. Presets SHLD preservam camadas de intensidade. O sfizz desta versão não suporta `loop_type=alternate`: os loops usam reprodução para frente com o crossfade definido pelo autor. Não há portamento/slide dedicado aos 808s no sampler.

## Licenças e origem

Frequency303 declara CC0 para seus packs: https://www.frequency303.com/samples/

SHLD/Ludovic Drolez declara CC0-1.0 para os packs gratuitos listados: https://drolez.com/blog/music/ableton-free-sound-packs.php . Os avisos originais foram preservados. SAMPLE-SOURCES.json registra páginas, downloads e hashes dos arquivos originais. MiniFreak 02 e 02b são complementares; arquivos sobrepostos não foram contados duas vezes. O preset VolcaIslands foi excluído porque faltam seus WAV nos downloads gratuitos.

Os WAV originais Instro desta expansão foram dedicados a CC0. O código continua com a licença do projeto. Packs que exigiam checkout ou cadastro ficaram de fora; não há samples comerciais redistribuídos.

## Verificação

Há relatórios técnicos de carregamento/renderização e uma demonstração WAV para você avaliar os timbres. Não foi possível testar dentro de uma sessão real do Ableton Live 12. A escolha artística da qualidade sonora deve ser feita ouvindo a demonstração e tocando os instrumentos.
