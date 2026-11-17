from pathlib import Path
from reportlab.pdfgen import canvas
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.lib.colors import HexColor
from reportlab.platypus import Paragraph
from reportlab.lib.styles import ParagraphStyle
import fitz
R=Path(__file__).resolve().parent; A=R/'assets'; OUT=R.parent/'Morph-Quick-Reference.pdf'
pdfmetrics.registerFont(TTFont('LS','/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf'))
pdfmetrics.registerFont(TTFont('LSB','/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf'))
W,H=842,595;BG='#121011';FG='#eeeeee';MUT='#b3b3b5';LINE='#454347'
c=canvas.Canvas(str(OUT),pagesize=(W,H));c.setTitle('Morph / Screens & Gestures - gui2d16');c.setAuthor('OP-XY CFW DEV')
page=0; records=[]
def text(x,y,s,size=11,bold=False,color=FG):
 c.setFillColor(HexColor(color));c.setFont('LSB' if bold else 'LS',size);c.drawString(x,H-y-size*.82,s);records.append(s)
def para(x,y,w,s,size=11,color=MUT,bold=False):
 st=ParagraphStyle('p',fontName='LSB' if bold else 'LS',fontSize=size,leading=size*1.32,textColor=HexColor(color))
 p=Paragraph(s,st);_,h=p.wrap(w,1000);assert y+h<551,(page,s,y+h);p.drawOn(c,x,H-y-h);records.append(s);return h

def line(x,y,x2,y2):c.setStrokeColor(HexColor(LINE));c.setLineWidth(.6);c.line(x,H-y,x2,H-y2)
def start(title,sub):
 global page
 page+=1;c.setFillColor(HexColor(BG));c.rect(0,0,W,H,fill=1,stroke=0)
 text(34,24,'MORPH / SCREENS & GESTURES',9,True,MUT);text(34,49,title,29,True);para(34,88,774,sub,11);line(34,112,808,112)
def end():
 line(34,559,808,559);text(34,573,'GUI2D16  /  Independent OP-XY reference  /  17 November 2026',8,color=MUT)
 text(770,573,f'{page:02d} / 12',8,color=MUT);c.showPage()
def screen(name,x,y,w,caption=None):
 h=w*220/480;c.drawImage(str(A/(name+'.png')),x,H-y-h,width=w,height=h)
 c.setStrokeColor(HexColor(LINE));c.rect(x,H-y-h,w,h,fill=0,stroke=1)
 if caption:para(x,y+h+7,w,caption,9)
 return h

def card(x,y,w,h,title,body):
 c.setStrokeColor(HexColor(LINE));c.setLineWidth(.7);c.roundRect(x,H-y-h,w,h,4,stroke=1,fill=0)
 text(x+14,y+13,title,14,True);para(x+14,y+37,w-28,body,10)
def icon(key,x,y):
 name='knob' if key.startswith('K') else 'track' if key.startswith('T') and key!='TEMPO' else 'shift' if key=='SHIFT' else 'tempo' if key=='TEMPO' else 'inst' if key=='INST' else None
 if name:c.drawImage(str(A/(name+'.png')),x,H-y-27,width=27,height=27)
 else:
  c.setStrokeColor(HexColor(LINE));c.roundRect(x+2,H-y-25,24,24,2,fill=0,stroke=1)
  text(x+6,y+8,key[1:] if key.startswith('M') else key[:3],8,color=MUT)
 text(x,y+30,key,8,color=MUT)
def gesture(x,y,w,keys,action,title,body,compact=False):
 for i,k in enumerate(keys):
  icon(k,x+i*43,y)
  if i:text(x+i*43-10,y+12,'/' if 'either' in action else '+',9,color=MUT)
 para(x,y+41,140,action,8,FG)
 tx=x+(150 if w>500 else 145 if compact else 94)
 text(tx,y+2,title,10.5 if compact else 12,True)
 para(tx,y+21,w-(tx-x),body,9 if compact else 10)

start('Morph / two scenes, one performance','Capture two versions of a sound, then move towards either one across the tracks you choose.')
c.drawImage(str(A/'morph_logo.png'),52,H-150-196,width=304,height=196)
text(424,139,'WHAT MORPH DOES',9,True,MUT)
para(424,164,384,'Morph stores two scenes, A and B, across all eight instrument tracks. Each scene holds supported instrument, envelope, filter and send settings. Move between sounds with an encoder, the pressure pad, MIDI or the LFO.',12,FG)
para(424,263,384,'AUX is the home for capture, routing, pad setup and presets. The instrument HUD lets you inspect and edit A/B beside the sound controls. S is a separate return snapshot; BYPASS returns enabled tracks to S.',11)
text(34,371,'READ THE SYMBOL',9,True,MUT)
card(34,394,246,89,'Filled circle / A','The first captured scene. Left in AUX; above in the instrument HUD.')
card(298,394,246,89,'Outline circle / B','The second captured scene. Right in AUX; below in the instrument HUD.')
card(562,394,246,89,'Small square / position','Shows sweep travel. The surrounding capsule appears only at the midpoint.')
para(34,504,774,'K1-K4 = encoders above the screen. M1-M4 = module buttons below it. T1-T8 = instrument track buttons.',10,FG)
para(34,528,774,'press = click once    hold = keep down    turn = rotate    + = overlap    then = ordered',10)
end()

start('M1 / capture & move','Open AUX, select the track holding Morph (normally AUX 4 / CV), then press M1.')
screen('aux1_live',34,135,350,'Filled circle = A. Outline = B. The small square shows sweep position.')
card(34,333,108,82,'A','Stored endpoint.')
card(155,333,108,82,'B','Stored endpoint.')
card(276,333,108,82,'S','Return snapshot.')
para(34,438,350,'First morph: prepare a sound and press K1. Change the sound and press K2. Turn K4 to move; press K4 to return to S.',12,FG)
para(34,508,350,'Icons flash twice after a press. Capture includes all eight instrument tracks.',9)
for y,args in zip([135,200,265,330,395,460],[(['K1'],'press','Capture A','All eight tracks.'),(['K2'],'press','Capture B','All eight tracks.'),(['K3'],'press','SET S','Explicitly store the return sound.'),(['K4'],'turn','Move toward A / B','Only enabled tracks follow the movement.'),(['K4'],'press','LIVE / BYPASS','BYPASS returns enabled tracks to S.'),(['K1'],'turn','Choose edit A / B','Left selects A; right selects B.')]):gesture(424,y,384,*args)
end()
start('M2 / routing     M4 / LFO','Capture includes every track. Routing decides which tracks Morph can change.')
screen('aux2',34,135,350,'M2 / encoder bank for tracks 1-4. White lower cells are ON; dark cells are OFF.')
screen('aux2_bank',34,338,350,'M2 / bank for tracks 5-8. Switching banks does not change the route states.')
for y,args in zip([135,217,299,381],[(['M2'],'press','Open routing','Eight columns represent the instrument tracks.'),(['K1'],'turn K1-K4','Enable / exclude','Turn the corresponding encoder right for ON, left for OFF.'),(['K1'],'press any encoder','Switch 1-4 / 5-8','The four shaded headers identify the current bank.'),(['M4'],'press','Open the stock LFO','Choose Morph, the fourth machine control, as destination. Use the stock waveform, rate and amount controls.')]):gesture(424,y,384,*args)
para(424,479,384,'LFO movement requires LIVE. Manual BYPASS stays bypassed. Start with slow modulation; fast modulation may sound stepped.',10,FG)
end()
start('M3 / sweep sensitivity & presets','Set pad and HUD encoder sensitivity independently. Enable LIVE before using the pad.')
screen('aux3',34,135,350,'Light Touch curve. Pad sensitivity is +200% on the horizontal axis and held-K4 sensitivity is +300% on the vertical axis.')
card(34,333,350,115,'Press, travel, release','Down/left moves toward A; up/right toward B. More pressure gives faster travel. Release holds position. The pad never enables LIVE itself.')
para(34,470,350,'The square plot shows Light Touch, LINEAR or EXP. Half pressure gives 80% speed with Light Touch, 50% with LINEAR and 15% with EXP. The arrows share an origin. White points right or up for faster travel. Red points left or down for finer control.',10)
rows=[(['K1'],'turn or press','PITCH / MORPH','Toggle stock pitch bend or Morph pad control. Release a held pad to finish switching.'),(['K2'],'turn','Speed sensitivity','At -90% the pad sweeps at one-tenth speed. At +200% it sweeps at triple speed. One more step selects Instant (infinity).'),(['K2'],'press','Cycle pressure curve','Light Touch > LINEAR > EXP. The drawn curve changes with the selection.'),(['K3'],'turn','Held K4 sensitivity','Adjust the HUD sweep in 5% steps. At -95%, twenty detents move one step. At +300%, each detent moves four.'),(['K4'],'turn','Choose preset 01-16','Scroll numerically. The filled cell is selected; the end slots clamp.'),(['K3'],'press','Load selected A/B','Requires BYPASS. Playback may continue.'),(['K4'],'press','Save / replace selected A/B','Saves immediately in BYPASS. No second confirmation press.')]
for i,args in enumerate(rows):gesture(424,130+i*59,384,*args)
end()
start('A/B presets / keep & recall','The sixteen slots store both scenes across all eight tracks. They do not contain S, samples or instrument installations.')
screen('aux3_live',34,135,350,'EXP curve. The red stripes show LIVE. Switch to BYPASS before saving or loading.')
card(34,333,350,114,'Save / load in BYPASS','Turn K4 to select a slot. Press K3 to load or K4 to save over the selected slot. Both scenes must be captured to save. The sequencer may keep playing.')
para(34,468,350,'A damaged or failed read does not partially apply a scene. Watch the status below the grid for the result.',11)
text(424,139,'INSTRUMENT COMPATIBILITY',9,True,MUT)
para(424,164,384,'Matching instruments recall supported scene parameters. For a mismatch, compatible M2/M3 fields load while the current M1 and engine-specific SHIFT values in A/B are preserved. S is never loaded from an A/B preset.',11)
text(424,272,'READ THE STATUS',9,True,MUT)
para(424,297,384,'Saved / Loaded: the operation completed.<br/>M1 preserved: a partial recall kept incompatible instrument-specific values.<br/>BYPASS required: switch out of LIVE before retrying.<br/>Capture A/B first: capture both scenes before saving.<br/>Empty or damaged / File/context error: recall or save could not complete.',11)
para(424,458,384,'Saving replaces the selected slot immediately. Check its number before pressing K4. The white selection fill identifies the slot in BYPASS.',11,FG)
end()

start('Instrument HUD / screen catalogue','Hold the selected instrument track for 150 ms. Release to leave, unless the HUD is latched.')
for x,name,caption in [(34,'hud1','M1 / instrument parameters and A-S-B rows'),(298,'hud2','M2 / AMP left, FILTER right; A above B'),(562,'hud3','M3 / filter model and A/B response outlines')]:screen(name,x,135,246,caption)
card(34,290,246,129,'A / B','Filled circle = A; outline = B. Red values or curves identify the scene being edited. In M2, only the selected scene/envelope is red.')
card(298,290,246,129,'S / return sound','M1 shows the stored return values in its S row. S follows stored instrument settings in BYPASS; LIVE freezes it except explicit SET S.')
card(562,290,246,129,'Movement','The right-hand sweep is vertical: A above, B below. Its square tracks travel. The centre is the movement origin, not necessarily S.')
para(34,444,774,'A white title background marks the selected bank. Displayed values are 0-99; the underlying parameters retain their full precision.',11,FG)
para(34,490,774,'While holding the track, the stock scramble/copy/paste/save menu disappears after 1.5 seconds. Its functions remain available. SHIFT before track keeps stock menus; track before SHIFT gives Morph gestures.',10)
end()
start('HUD / filters & SHIFT','Use the current filter model and the stock icons to read each parameter in context.')
screen('hud3_dynamic',34,135,246,'Dynamic / independent low-cut and high-cut edges')
screen('hud2_shift',298,135,246,'M2 + SHIFT / play mode, glide, bend and level')
screen('hud3_shift',562,135,246,'M3 + SHIFT / AUX out, tape, FX I and FX II')
text(34,292,'DYNAMIC FILTER',9,True,MUT)
for i,(k,t) in enumerate([('K1','Low cut / high-pass cutoff'),('K2','Compression amount (C)'),('K3','Distortion amount (D)'),('K4','High cut / low-pass cutoff')]):
 y=319+i*34;text(34,y,k,11,True);para(72,y,208,t,10);line(34,y+25,280,y+25)
para(34,470,246,'C and D bars show parameter amounts, not live audio meters. The band outline is a control-position schematic.',10)
text(298,292,'READ THE COLUMNS',9,True,MUT)
para(298,317,246,'SHIFT tables show the current value, then A and B. Red marks the edited scene. Dashes mean there is no supported scene value.',11)
para(298,391,246,'On M2 SHIFT, Morph edits glide with K2 and preset level with K4. Play mode and bend range are current-only readouts.',11)
para(298,478,246,'Engine-specific M1 SHIFT fields remain read-only in the HUD.',10)
text(562,292,'OTHER FILTER MODELS',9,True,MUT)
para(562,317,246,'Low-pass and high-pass use their respective outlines. Cutoff and resonance values move with their graphical controls; the side values are envelope amount and key tracking.',11)
para(562,414,246,'M3 SHIFT edits the four send levels for the selected A/B scene. Use BYPASS for snapshot edits.',11)
para(562,478,246,'Hold the instrument track first, then SHIFT, before using Morph SHIFT gestures.',10,FG)
end()
start('HUD / button combinations','For held-HUD SHIFT gestures: hold the instrument track first, then SHIFT, then the encoder.')
left=[(['T1'],'hold','Hold-to-open HUD','Use the selected T1-T8. Release to close.'),(['SHIFT','TEMPO'],'hold + press','Latch / unlatch HUD','Hold SHIFT, then press TEMPO.'),(['INST'],'press','Clear the HUD latch','Return to the normal instrument page.'),(['K1'],'press','Alternate page / bank','M1: available alternate bank. M2: AMP / FILTER.'),(['K2'],'press','Choose edit A / B','Select the scene the encoders edit.'),(['K1'],'turn K1-K4','Edit snapshot values','In BYPASS, turn for the visible fields.'),(['K3'],'press','SET S','All eight tracks, independent of the edit scene.')]
right=[(['K4'],'press','LIVE / BYPASS','Movement and return to S affect enabled tracks.'),(['K4'],'hold + turn','Sweep','Keep K4 held while turning K4.'),(['SHIFT','K1'],'hold + press','Capture this track into A','Selected instrument track only.'),(['SHIFT','K2'],'hold + press','Capture this track into B','Selected instrument track only.'),(['SHIFT','K3'],'hold + release','Jump toward A','Release K3 while SHIFT remains held.'),(['SHIFT','K4'],'hold + release','Jump toward B','Release K4 while SHIFT remains held.'),(['SHIFT','K3','K4'],'overlap','LIVE / BYPASS chord','Overlap K3 and K4; individual jumps are suppressed.')]
for x,rows in [(34,left),(430,right)]:
 for i,args in enumerate(rows):gesture(x,135+i*57,378,*args,compact=True)
end()
start('Read the sweep / A, centre & B','The square shows travel towards a scene. The midpoint is the movement origin, not necessarily the stored return S.')
text(34,139,'LIVE / A, MIDPOINT AND B',9,True,MUT)
for column,(i,label) in enumerate([(0,'At A'),(2,'Midpoint'),(4,'At B')]):
 x=34+column*264
 c.drawImage(str(A/f'sweep_mark_{i}.png'),x,H-191-74.3125,width=246,height=74.3125)
 text(x+98,294,label,11,True)
para(34,346,774,'Read the position of the small square. A is always the filled circle; B is always the outline. The circles separate as you travel away from the middle. The outer capsule appears only at the exact midpoint.',12,FG)
card(34,413,246,108,'AUX / horizontal','Left is A; right is B. Turn K4 on AUX M1 to sweep. The examples above show both endpoints and the midpoint.')
card(298,413,246,108,'HUD / vertical','The same display is turned upright: A above, B below. Hold K4 while turning to sweep from the instrument HUD.')
card(562,413,246,108,'BYPASS / return to S','Red stripes and a grey square identify BYPASS. S is a stored return sound, not the sweep marker or a guaranteed A/B midpoint.')
end()

start('Record, return, remember','Only physical movement on AUX M1 K4 is recorded as Morph automation.')
gesture(34,135,774,['REC','K4'],'hold + turn','Record movement','While the sequencer plays, hold RECORD and turn K4 on Morph AUX M1. Release RECORD; keep LIVE enabled for replay.')
gesture(34,220,774,['K4'],'press','Return to S / manual bypass','Recorded movement and the LFO cannot automatically re-enable LIVE. Press K4 explicitly to enable it again.')
gesture(34,305,774,['BAR','M2'],'hold BAR, then press M2','Erase parameter automation','On Morph AUX, clear ALL parameter locks in this AUX pattern. Notes and the other tracks are retained.')
card(34,400,246,120,'Save / reload','Keep project.xy and its .oxy companion together. The companion stores snapshots, pad settings and the recording origin. Enable LIVE manually after loading.')
card(298,400,246,120,'New context','Only the current snapshot bank is kept. Engine/schema, pattern or scene changes start a fresh bank. Switching back does not recall unsaved banks.')
card(562,400,246,120,'Everyday behaviour','Startup captures once; AUX revisits do not. Capturing while LIVE returns enabled tracks to S. Scramble retains A/B and refreshes the return/origin context.')
end()
start('Movement sources / quick decisions','Pad and MIDI are manual movement sources. Only physical AUX M1 K4 movement is recorded.')
screen('aux1_bypass',34,135,350,'BYPASS / red striping and a grey square; no text label is needed.')
card(34,333,350,111,'Read the sweep','In AUX, A is left and B is right. In the HUD, A is above B. The outer capsule appears only at the exact midpoint. Centre is the movement origin, not necessarily S.')
para(34,469,350,'Excluded tracks stay at their current sound, including on return to S. Their A/B values remain capturable and editable.',11)
text(424,139,'PHYSICAL ENCODER',9,True,MUT)
para(424,164,384,'AUX M1 K4 is the recordable movement source. In the HUD, hold K4 and turn to sweep. Set its sensitivity with AUX M3 K3. New projects start at +100%. Companions from before gui2d15 open at 0%.',11)
text(424,233,'ONBOARD PAD',9,True,MUT)
para(424,258,384,'MORPH consumes the physical pad pitch bend. Incoming MIDI pitch bend keeps its stock route. After mute, bypass or another source takes over, release the pad before pressing again.',11)
text(424,344,'MIDI MOVEMENT',9,True,MUT)
para(424,369,384,'Receive channel 12, CC15: 0 = A; 63 and 64 = movement origin; 127 = B. Enable LIVE first. MIDI movement is not recorded as Morph automation.',11)
text(424,455,'WHICH SOURCE WINS?',9,True,MUT)
para(424,480,384,'Physical movement or a new CC15 message can take over the pad. Pad/MIDI takeover holds off LFO and recorded movement. Manual BYPASS stays bypassed.',10)
end()
start('Keep your setup / projects & Manager','Save sounds and keep a known working setup before changing the installed instruments.')
card(34,135,350,128,'Project + companion','Keep project.xy and its .oxy companion together. The companion preserves Morph snapshots, pad and HUD sensitivity settings, and the recording origin. Enable LIVE manually after loading.')
card(34,287,350,128,'Keep rollback copies','Retain original projects and companions when updating. Older releases may reject newer saved banks. A/B presets store settings; keep the required instruments and samples too.')
para(34,444,350,'Guide edition: gui2d16. Screen examples come from the current renderer with illustrative values and a substitute preview font. General screen examples show the midpoint; the sweep page shows several positions.',10)
text(424,139,'UPDATE MORPH',9,True,MUT)
para(424,164,384,'In Manager, import the new Morph source and select Replace installed Morph. Use the compatible SDK edition, select the correct device profile and scan its installed setup. Build and review before staging.',11)
text(424,278,'REMOVE A MACHINE',9,True,MUT)
para(424,303,384,'Select its row in Installed setup, then Remove selected installed machines. Keep selected installed machines reverses the selection; Clear changes cancels pending edits.',11)
para(424,386,384,'Calculate, build and review before staging. Projects and presets remain, but projects using a removed machine need it reinstalled.',11)
para(424,468,384,"Use each XY's own device profile and installed-setup scan when managing more than one unit.",11,FG)
end()

c.save()
(R/'guide_text.txt').write_text('\n\n'.join(records))
# Render every page for visual validation.
doc=fitz.open(OUT)
for i,p in enumerate(doc):p.get_pixmap(matrix=fitz.Matrix(1.2,1.2)).save(str(R/f'page-{i+1:02d}.png'))
print(OUT,len(doc),'pages')
