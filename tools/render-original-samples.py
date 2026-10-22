from pathlib import Path
import math, wave, json, sys
root=Path(sys.argv[1]) if len(sys.argv)>1 else Path('Instro-Urban'); folder=root/'Banks/Urban/Instro-Originals'; folder.mkdir(exist_ok=True)
catalog=json.loads((root/'Banks/catalog.json').read_text(encoding='utf-8')) if (root/'Banks/catalog.json').exists() else []; catalog=[x for x in catalog if x['source']!='Instro - sintese original renderizada em WAV']
configs=[('808 Clean',0.0,2.0,0),('808 Short',0.0,.55,0),('808 Long',0.0,3.6,0),('808 Saturated',2.0,1.7,0),('808 Hard',4.0,1.1,0),('808 Soft',.4,2.3,0),('808 Fifth',1.0,1.8,.25),('808 Octave',.8,1.8,.5),('Bell Glass',0,1.2,2),('Bell Dark',0,1.8,3),('Pluck Digital',0,.65,4),('Pluck Soft',0,1.0,5)]
for name,drive,decay,kind in configs:
 regions=[]; bass=name.startswith('808'); keys=[24,36,48] if bass else [48,60,72,84]
 for i,key in enumerate(keys):
  rate=48000; seconds=8 if bass else 6; freq=440*2**((key-69)/12); audio=bytearray()
  for n in range(int(rate*seconds)):
   t=n/rate
   if bass:
    # Pitch falls only during the first milliseconds; sustained fundamental is tuned.
    phase=2*math.pi*freq*(t+.022*(1-math.exp(-t/.012)))
    value=math.sin(phase)
    if kind==.25: value+=.25*math.sin(phase*1.5)
    if kind==.5: value+=.3*math.sin(phase*2)
    if drive: value=math.tanh(value*(1+drive))/math.tanh(1+drive)
   else:
    phase=2*math.pi*freq*t
    if kind==2: value=math.sin(phase+2.4*math.exp(-t*3)*math.sin(phase*2))
    elif kind==3: value=math.sin(phase+.9*math.exp(-t*2)*math.sin(phase*3.5))
    elif kind==4: value=(math.sin(phase)+.45*math.exp(-t*8)*math.sin(phase*3)+.2*math.exp(-t*12)*math.sin(phase*5))/1.65
    else: value=math.sin(phase+.6*math.exp(-t*7)*math.sin(phase*2))
   env=(1-math.exp(-t*1000))*math.exp(-t/decay)*min(1,(seconds-t)/.05)
   sample=round(max(-1,min(1,value*env*.75))*8388607)
   audio.extend(sample.to_bytes(3,'little',signed=True))
  file=folder/f'{name}-{key}.wav'
  with wave.open(str(file),'wb') as w: w.setnchannels(1); w.setsampwidth(3); w.setframerate(rate); w.writeframes(audio)
  low=0 if i==0 else (keys[i-1]+key)//2+1; high=127 if i==len(keys)-1 else (key+keys[i+1])//2
  regions.append(f'<region> sample={file.name} pitch_keycenter={key} lokey={low} hikey={high}')
 sfz=folder/f'{name}.sfz'; sfz.write_text('<global> ampeg_release=0.12 loop_mode=no_loop volume=-3\n'+'\n'.join(regions),encoding='utf-8')
 catalog.append(dict(family='Trap - 808' if bass else 'Synth - Bells e plucks',instrument=name,articulation='Original Instro - 24 bit / 48 kHz',path=sfz.relative_to(root/'Banks').as_posix(),source='Instro - sintese original renderizada em WAV',license='CC0-1.0',testNote=36 if bass else 60,testVelocity=100))
(folder/'LICENSE.txt').write_text('Original procedural audio created for Instro. Dedicated to the public domain under CC0-1.0. https://creativecommons.org/publicdomain/zero/1.0/\nThese are original synthesized samples, not recordings of a Roland TR-808.\n',encoding='utf-8')
(root/'Banks/catalog.json').write_text(json.dumps(catalog,indent=2,ensure_ascii=False),encoding='utf-8')
old=[]
(root/'catalog-completo.json').write_text(json.dumps(old+catalog,indent=2,ensure_ascii=False),encoding='utf-8')
print('Added',len(configs),'original instruments; total new',len(catalog))
