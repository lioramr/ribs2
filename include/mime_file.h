#ifndef __MIME_FILE__H_
#define __MIME_FILE__H_

/* macs don't have the /etc/mime.types file.
   This string is modified in mime_types init */
char mime_file[] = "application/activemessage\n\
application/andrew-inset	ez\n\
application/annodex	anx\n\
application/applefile\n\
application/atom+xml	atom\n\
application/atomcat+xml	atomcat\n\
application/atomicmail\n\
application/atomserv+xml	atomsrv\n\
application/batch-SMTP\n\
application/bbolin	lin\n\
application/beep+xml\n\
application/cals-1840\n\
application/cap	cap pcap\n\
application/commonground\n\
application/cu-seeme	cu\n\
application/cybercash\n\
application/davmount+xml	davmount\n\
application/dca-rft\n\
application/dec-dx\n\
application/docbook+xml\n\
application/dsptype	tsp\n\
application/dvcs\n\
application/ecmascript	es\n\
application/edi-consent\n\
application/edi-x12\n\
application/edifact\n\
application/eshop\n\
application/font-tdpfr\n\
application/futuresplash	spl\n\
application/ghostview\n\
application/hta	hta\n\
application/http\n\
application/hyperstudio\n\
application/iges\n\
application/index\n\
application/index.cmd\n\
application/index.obj\n\
application/index.response\n\
application/index.vnd\n\
application/iotp\n\
application/ipp\n\
application/isup\n\
application/java-archive	jar\n\
application/java-serialized-object	ser\n\
application/java-vm	class\n\
application/javascript	js\n\
application/json	json\n\
application/m3g	m3g\n\
application/mac-binhex40	hqx\n\
application/mac-compactpro	cpt\n\
application/macwriteii\n\
application/marc\n\
application/mathematica	nb nbp\n\
application/ms-tnef\n\
application/msaccess	mdb\n\
application/msword	doc dot\n\
application/mxf	mxf\n\
application/news-message-id\n\
application/news-transmission\n\
application/ocsp-request\n\
application/ocsp-response\n\
application/octet-stream	bin\n\
application/oda	oda\n\
application/ogg	ogx\n\
application/onenote	one onetoc2 onetmp onepkg\n\
application/parityfec\n\
application/pdf	pdf\n\
application/pgp-encrypted\n\
application/pgp-keys	key\n\
application/pgp-signature	pgp\n\
application/pics-rules	prf\n\
application/pkcs10\n\
application/pkcs7-mime\n\
application/pkcs7-signature\n\
application/pkix-cert\n\
application/pkix-crl\n\
application/pkixcmp\n\
application/postscript	ps ai eps epsi epsf eps2 eps3\n\
application/prs.alvestrand.titrax-sheet\n\
application/prs.cww\n\
application/prs.nprend\n\
application/qsig\n\
application/rar	rar\n\
application/rdf+xml	rdf\n\
application/remote-printing\n\
application/riscos\n\
application/rss+xml	rss\n\
application/rtf	rtf\n\
application/sdp\n\
application/set-payment\n\
application/set-payment-initiation\n\
application/set-registration\n\
application/set-registration-initiation\n\
application/sgml\n\
application/sgml-open-catalog\n\
application/sieve\n\
application/sla	stl\n\
application/slate\n\
application/smil	smi smil\n\
application/timestamp-query\n\
application/timestamp-reply\n\
application/vemmi\n\
application/whoispp-query\n\
application/whoispp-response\n\
application/wita\n\
application/x400-bp\n\
application/xhtml+xml	xhtml xht\n\
application/xml	xml xsl xsd\n\
application/xml-dtd\n\
application/xml-external-parsed-entity\n\
application/xspf+xml	xspf\n\
application/zip	zip\n\
application/vnd.3M.Post-it-Notes\n\
application/vnd.accpac.simply.aso\n\
application/vnd.accpac.simply.imp\n\
application/vnd.acucobol\n\
application/vnd.aether.imp\n\
application/vnd.android.package-archive	apk\n\
application/vnd.anser-web-certificate-issue-initiation\n\
application/vnd.anser-web-funds-transfer-initiation\n\
application/vnd.audiograph\n\
application/vnd.bmi\n\
application/vnd.businessobjects\n\
application/vnd.canon-cpdl\n\
application/vnd.canon-lips\n\
application/vnd.cinderella	cdy\n\
application/vnd.claymore\n\
application/vnd.commerce-battelle\n\
application/vnd.commonspace\n\
application/vnd.comsocaller\n\
application/vnd.contact.cmsg\n\
application/vnd.cosmocaller\n\
application/vnd.ctc-posml\n\
application/vnd.cups-postscript\n\
application/vnd.cups-raster\n\
application/vnd.cups-raw\n\
application/vnd.cybank\n\
application/vnd.dna\n\
application/vnd.dpgraph\n\
application/vnd.dxr\n\
application/vnd.ecdis-update\n\
application/vnd.ecowin.chart\n\
application/vnd.ecowin.filerequest\n\
application/vnd.ecowin.fileupdate\n\
application/vnd.ecowin.series\n\
application/vnd.ecowin.seriesrequest\n\
application/vnd.ecowin.seriesupdate\n\
application/vnd.enliven\n\
application/vnd.epson.esf\n\
application/vnd.epson.msf\n\
application/vnd.epson.quickanime\n\
application/vnd.epson.salt\n\
application/vnd.epson.ssf\n\
application/vnd.ericsson.quickcall\n\
application/vnd.eudora.data\n\
application/vnd.fdf\n\
application/vnd.ffsns\n\
application/vnd.flographit\n\
application/vnd.framemaker\n\
application/vnd.fsc.weblaunch\n\
application/vnd.fujitsu.oasys\n\
application/vnd.fujitsu.oasys2\n\
application/vnd.fujitsu.oasys3\n\
application/vnd.fujitsu.oasysgp\n\
application/vnd.fujitsu.oasysprs\n\
application/vnd.fujixerox.ddd\n\
application/vnd.fujixerox.docuworks\n\
application/vnd.fujixerox.docuworks.binder\n\
application/vnd.fut-misnet\n\
application/vnd.google-earth.kml+xml	kml\n\
application/vnd.google-earth.kmz	kmz\n\
application/vnd.grafeq\n\
application/vnd.groove-account\n\
application/vnd.groove-identity-message\n\
application/vnd.groove-injector\n\
application/vnd.groove-tool-message\n\
application/vnd.groove-tool-template\n\
application/vnd.groove-vcard\n\
application/vnd.hhe.lesson-player\n\
application/vnd.hp-HPGL\n\
application/vnd.hp-PCL\n\
application/vnd.hp-PCLXL\n\
application/vnd.hp-hpid\n\
application/vnd.hp-hps\n\
application/vnd.httphone\n\
application/vnd.hzn-3d-crossword\n\
application/vnd.ibm.MiniPay\n\
application/vnd.ibm.afplinedata\n\
application/vnd.ibm.modcap\n\
application/vnd.informix-visionary\n\
application/vnd.intercon.formnet\n\
application/vnd.intertrust.digibox\n\
application/vnd.intertrust.nncp\n\
application/vnd.intu.qbo\n\
application/vnd.intu.qfx\n\
application/vnd.irepository.package+xml\n\
application/vnd.is-xpr\n\
application/vnd.japannet-directory-service\n\
application/vnd.japannet-jpnstore-wakeup\n\
application/vnd.japannet-payment-wakeup\n\
application/vnd.japannet-registration\n\
application/vnd.japannet-registration-wakeup\n\
application/vnd.japannet-setstore-wakeup\n\
application/vnd.japannet-verification\n\
application/vnd.japannet-verification-wakeup\n\
application/vnd.koan\n\
application/vnd.lotus-1-2-3\n\
application/vnd.lotus-approach\n\
application/vnd.lotus-freelance\n\
application/vnd.lotus-notes\n\
application/vnd.lotus-organizer\n\
application/vnd.lotus-screencam\n\
application/vnd.lotus-wordpro\n\
application/vnd.mcd\n\
application/vnd.mediastation.cdkey\n\
application/vnd.meridian-slingshot\n\
application/vnd.mif\n\
application/vnd.minisoft-hp3000-save\n\
application/vnd.mitsubishi.misty-guard.trustweb\n\
application/vnd.mobius.daf\n\
application/vnd.mobius.dis\n\
application/vnd.mobius.msl\n\
application/vnd.mobius.plc\n\
application/vnd.mobius.txf\n\
application/vnd.motorola.flexsuite\n\
application/vnd.motorola.flexsuite.adsi\n\
application/vnd.motorola.flexsuite.fis\n\
application/vnd.motorola.flexsuite.gotap\n\
application/vnd.motorola.flexsuite.kmr\n\
application/vnd.motorola.flexsuite.ttc\n\
application/vnd.motorola.flexsuite.wem\n\
application/vnd.mozilla.xul+xml	xul\n\
application/vnd.ms-artgalry\n\
application/vnd.ms-asf\n\
application/vnd.ms-excel	xls xlb xlt\n\
application/vnd.ms-excel.addin.macroEnabled.12	xlam\n\
application/vnd.ms-excel.sheet.binary.macroEnabled.12	xlsb\n\
application/vnd.ms-excel.sheet.macroEnabled.12	xlsm\n\
application/vnd.ms-excel.template.macroEnabled.12	xltm\n\
application/vnd.ms-lrm\n\
application/vnd.ms-officetheme	thmx\n\
application/vnd.ms-pki.seccat	cat\n\
\n\
application/vnd.ms-powerpoint	ppt pps\n\
application/vnd.ms-powerpoint.addin.macroEnabled.12	ppam\n\
application/vnd.ms-powerpoint.presentation.macroEnabled.12	pptm\n\
application/vnd.ms-powerpoint.slide.macroEnabled.12	sldm\n\
application/vnd.ms-powerpoint.slideshow.macroEnabled.12	ppsm\n\
application/vnd.ms-powerpoint.template.macroEnabled.12	potm\n\
application/vnd.ms-project\n\
application/vnd.ms-tnef\n\
application/vnd.ms-word.document.macroEnabled.12	docm\n\
application/vnd.ms-word.template.macroEnabled.12	dotm\n\
application/vnd.ms-works\n\
application/vnd.mseq\n\
application/vnd.msign\n\
application/vnd.music-niff\n\
application/vnd.musician\n\
application/vnd.netfpx\n\
application/vnd.noblenet-directory\n\
application/vnd.noblenet-sealer\n\
application/vnd.noblenet-web\n\
application/vnd.novadigm.EDM\n\
application/vnd.novadigm.EDX\n\
application/vnd.novadigm.EXT\n\
application/vnd.oasis.opendocument.chart	odc\n\
application/vnd.oasis.opendocument.database	odb\n\
application/vnd.oasis.opendocument.formula	odf\n\
application/vnd.oasis.opendocument.graphics	odg\n\
application/vnd.oasis.opendocument.graphics-template	otg\n\
application/vnd.oasis.opendocument.image	odi\n\
application/vnd.oasis.opendocument.presentation	odp\n\
application/vnd.oasis.opendocument.presentation-template	otp\n\
application/vnd.oasis.opendocument.spreadsheet	ods\n\
application/vnd.oasis.opendocument.spreadsheet-template	ots\n\
application/vnd.oasis.opendocument.text	odt\n\
application/vnd.oasis.opendocument.text-master	odm\n\
application/vnd.oasis.opendocument.text-template	ott\n\
application/vnd.oasis.opendocument.text-web	oth\n\
application/vnd.openxmlformats-officedocument.presentationml.presentation	pptx\n\
application/vnd.openxmlformats-officedocument.presentationml.slide	sldx\n\
application/vnd.openxmlformats-officedocument.presentationml.slideshow	ppsx\n\
application/vnd.openxmlformats-officedocument.presentationml.template	potx\n\
application/vnd.openxmlformats-officedocument.spreadsheetml.sheet	xlsx\n\
application/vnd.openxmlformats-officedocument.spreadsheetml.sheet	xlsx\n\
application/vnd.openxmlformats-officedocument.spreadsheetml.template	xltx\n\
application/vnd.openxmlformats-officedocument.spreadsheetml.template	xltx\n\
application/vnd.openxmlformats-officedocument.wordprocessingml.document	docx\n\
application/vnd.openxmlformats-officedocument.wordprocessingml.template	dotx\n\
application/vnd.osa.netdeploy\n\
application/vnd.palm\n\
application/vnd.pg.format\n\
application/vnd.pg.osasli\n\
application/vnd.powerbuilder6\n\
application/vnd.powerbuilder6-s\n\
application/vnd.powerbuilder7\n\
application/vnd.powerbuilder7-s\n\
application/vnd.powerbuilder75\n\
application/vnd.powerbuilder75-s\n\
application/vnd.previewsystems.box\n\
application/vnd.publishare-delta-tree\n\
application/vnd.pvi.ptid1\n\
application/vnd.pwg-xhtml-print+xml\n\
application/vnd.rapid\n\
application/vnd.rim.cod	cod\n\
application/vnd.s3sms\n\
application/vnd.seemail\n\
application/vnd.shana.informed.formdata\n\
application/vnd.shana.informed.formtemplate\n\
application/vnd.shana.informed.interchange\n\
application/vnd.shana.informed.package\n\
application/vnd.smaf	mmf\n\
application/vnd.sss-cod\n\
application/vnd.sss-dtf\n\
application/vnd.sss-ntf\n\
application/vnd.stardivision.calc	sdc\n\
application/vnd.stardivision.chart	sds\n\
application/vnd.stardivision.draw	sda\n\
application/vnd.stardivision.impress	sdd\n\
application/vnd.stardivision.math	sdf\n\
application/vnd.stardivision.writer	sdw\n\
application/vnd.stardivision.writer-global	sgl\n\
application/vnd.street-stream\n\
application/vnd.sun.xml.calc	sxc\n\
application/vnd.sun.xml.calc.template	stc\n\
application/vnd.sun.xml.draw	sxd\n\
application/vnd.sun.xml.draw.template	std\n\
application/vnd.sun.xml.impress	sxi\n\
application/vnd.sun.xml.impress.template	sti\n\
application/vnd.sun.xml.math	sxm\n\
application/vnd.sun.xml.writer	sxw\n\
application/vnd.sun.xml.writer.global	sxg\n\
application/vnd.sun.xml.writer.template	stw\n\
application/vnd.svd\n\
application/vnd.swiftview-ics\n\
application/vnd.symbian.install	sis\n\
application/vnd.triscape.mxs\n\
application/vnd.trueapp\n\
application/vnd.truedoc\n\
application/vnd.tve-trigger\n\
application/vnd.ufdl\n\
application/vnd.uplanet.alert\n\
application/vnd.uplanet.alert-wbxml\n\
application/vnd.uplanet.bearer-choice\n\
application/vnd.uplanet.bearer-choice-wbxml\n\
application/vnd.uplanet.cacheop\n\
application/vnd.uplanet.cacheop-wbxml\n\
application/vnd.uplanet.channel\n\
application/vnd.uplanet.channel-wbxml\n\
application/vnd.uplanet.list\n\
application/vnd.uplanet.list-wbxml\n\
application/vnd.uplanet.listcmd\n\
application/vnd.uplanet.listcmd-wbxml\n\
application/vnd.uplanet.signal\n\
application/vnd.vcx\n\
application/vnd.vectorworks\n\
application/vnd.vidsoft.vidconference\n\
application/vnd.visio	vsd\n\
application/vnd.vividence.scriptfile\n\
application/vnd.wap.sic\n\
application/vnd.wap.slc\n\
application/vnd.wap.wbxml	wbxml\n\
application/vnd.wap.wmlc	wmlc\n\
application/vnd.wap.wmlscriptc	wmlsc\n\
application/vnd.webturbo\n\
application/vnd.wordperfect	wpd\n\
application/vnd.wordperfect5.1	wp5\n\
application/vnd.wrq-hp3000-labelled\n\
application/vnd.wt.stf\n\
application/vnd.xara\n\
application/vnd.xfdl\n\
application/vnd.yellowriver-custom-menu\n\
application/x-123	wk\n\
application/x-7z-compressed	7z\n\
application/x-abiword	abw\n\
application/x-apple-diskimage	dmg\n\
application/x-bcpio	bcpio\n\
application/x-bittorrent	torrent\n\
application/x-cab	cab\n\
application/x-cbr	cbr\n\
application/x-cbz	cbz\n\
application/x-cdf	cdf cda\n\
application/x-cdlink	vcd\n\
application/x-chess-pgn	pgn\n\
application/x-comsol	mph\n\
application/x-core\n\
application/x-cpio	cpio\n\
application/x-csh	csh\n\
application/x-debian-package	deb udeb\n\
application/x-director	dcr dir dxr\n\
application/x-dms	dms\n\
application/x-doom	wad\n\
application/x-dvi	dvi\n\
application/x-executable\n\
application/x-font	pfa pfb gsf pcf pcf.Z\n\
application/x-freemind	mm\n\
application/x-futuresplash	spl\n\
application/x-ganttproject	gan\n\
application/x-gnumeric	gnumeric\n\
application/x-go-sgf	sgf\n\
application/x-graphing-calculator	gcf\n\
application/x-gtar	gtar\n\
application/x-gtar-compressed	tgz taz\n\
application/x-hdf	hdf\n\
application/x-httpd-eruby	rhtml\n\
application/x-httpd-php	phtml pht php\n\
application/x-httpd-php-source	phps\n\
application/x-httpd-php3	php3\n\
application/x-httpd-php3-preprocessed	php3p\n\
application/x-httpd-php4	php4\n\
application/x-httpd-php5	php5\n\
application/x-ica	ica\n\
application/x-info	info\n\
application/x-internet-signup	ins isp\n\
application/x-iphone	iii\n\
application/x-iso9660-image	iso\n\
application/x-jam	jam\n\
application/x-java-applet\n\
application/x-java-bean\n\
application/x-java-jnlp-file	jnlp\n\
application/x-jmol	jmz\n\
application/x-kchart	chrt\n\
application/x-kdelnk\n\
application/x-killustrator	kil\n\
application/x-koan	skp skd skt skm\n\
application/x-kpresenter	kpr kpt\n\
application/x-kspread	ksp\n\
application/x-kword	kwd kwt\n\
application/x-latex	latex\n\
application/x-lha	lha\n\
application/x-lyx	lyx\n\
application/x-lzh	lzh\n\
application/x-lzx	lzx\n\
application/x-maker	frm maker frame fm fb book fbdoc\n\
application/x-mif	mif\n\
application/x-mpegURL	m3u8\n\
application/x-ms-wmd	wmd\n\
application/x-ms-wmz	wmz\n\
application/x-msdos-program	com exe bat dll\n\
application/x-msi	msi\n\
application/x-netcdf	nc\n\
application/x-ns-proxy-autoconfig	pac dat\n\
application/x-nwc	nwc\n\
application/x-object	o\n\
application/x-oz-application	oza\n\
application/x-pkcs7-certreqresp	p7r\n\
application/x-pkcs7-crl	crl\n\
application/x-python-code	pyc pyo\n\
application/x-qgis	qgs shp shx\n\
application/x-quicktimeplayer	qtl\n\
application/x-rdp	rdp\n\
application/x-redhat-package-manager	rpm\n\
application/x-ruby	rb\n\
application/x-rx\n\
application/x-scilab	sci sce\n\
application/x-sh	sh\n\
application/x-shar	shar\n\
application/x-shellscript\n\
application/x-shockwave-flash	swf swfl\n\
application/x-silverlight	scr\n\
application/x-sql	sql\n\
application/x-stuffit	sit sitx\n\
application/x-sv4cpio	sv4cpio\n\
application/x-sv4crc	sv4crc\n\
application/x-tar	tar\n\
application/x-tcl	tcl\n\
application/x-tex-gf	gf\n\
application/x-tex-pk	pk\n\
application/x-texinfo	texinfo texi\n\
application/x-trash	~ % bak old sik\n\
application/x-troff	t tr roff\n\
application/x-troff-man	man\n\
application/x-troff-me	me\n\
application/x-troff-ms	ms\n\
application/x-ustar	ustar\n\
application/x-videolan\n\
application/x-wais-source	src\n\
application/x-wingz	wz\n\
application/x-x509-ca-cert	crt\n\
application/x-xcf	xcf\n\
application/x-xfig	fig\n\
application/x-xpinstall	xpi\n\
\n\
audio/32kadpcm\n\
audio/3gpp\n\
audio/amr	amr\n\
audio/amr-wb	awb\n\
audio/amr	amr\n\
audio/amr-wb	awb\n\
audio/annodex	axa\n\
audio/basic	au snd\n\
audio/csound	csd orc sco\n\
audio/flac	flac\n\
audio/g.722.1\n\
audio/l16\n\
audio/midi	mid midi kar\n\
audio/mp4a-latm\n\
audio/mpa-robust\n\
audio/mpeg	mpga mpega mp2 mp3 m4a\n\
audio/mpegurl	m3u\n\
audio/ogg	oga ogg spx\n\
audio/parityfec\n\
audio/prs.sid	sid\n\
audio/telephone-event\n\
audio/tone\n\
audio/vnd.cisco.nse\n\
audio/vnd.cns.anp1\n\
audio/vnd.cns.inf1\n\
audio/vnd.digital-winds\n\
audio/vnd.everad.plj\n\
audio/vnd.lucent.voice\n\
audio/vnd.nortel.vbk\n\
audio/vnd.nuera.ecelp4800\n\
audio/vnd.nuera.ecelp7470\n\
audio/vnd.nuera.ecelp9600\n\
audio/vnd.octel.sbc\n\
audio/vnd.qcelp\n\
audio/vnd.rhetorex.32kadpcm\n\
audio/vnd.vmx.cvsd\n\
audio/x-aiff	aif aiff aifc\n\
audio/x-gsm	gsm\n\
audio/x-mpegurl	m3u\n\
audio/x-ms-wma	wma\n\
audio/x-ms-wax	wax\n\
audio/x-pn-realaudio-plugin\n\
audio/x-pn-realaudio	ra rm ram\n\
audio/x-realaudio	ra\n\
audio/x-scpls	pls\n\
audio/x-sd2	sd2\n\
audio/x-wav	wav\n\
\n\
chemical/x-alchemy	alc\n\
chemical/x-cache	cac cache\n\
chemical/x-cache-csf	csf\n\
chemical/x-cactvs-binary	cbin cascii ctab\n\
chemical/x-cdx	cdx\n\
chemical/x-cerius	cer\n\
chemical/x-chem3d	c3d\n\
chemical/x-chemdraw	chm\n\
chemical/x-cif	cif\n\
chemical/x-cmdf	cmdf\n\
chemical/x-cml	cml\n\
chemical/x-compass	cpa\n\
chemical/x-crossfire	bsd\n\
chemical/x-csml	csml csm\n\
chemical/x-ctx	ctx\n\
chemical/x-cxf	cxf cef\n\
\n\
chemical/x-embl-dl-nucleotide	emb embl\n\
chemical/x-galactic-spc	spc\n\
chemical/x-gamess-input	inp gam gamin\n\
chemical/x-gaussian-checkpoint	fch fchk\n\
chemical/x-gaussian-cube	cub\n\
chemical/x-gaussian-input	gau gjc gjf\n\
chemical/x-gaussian-log	gal\n\
chemical/x-gcg8-sequence	gcg\n\
chemical/x-genbank	gen\n\
chemical/x-hin	hin\n\
chemical/x-isostar	istr ist\n\
chemical/x-jcamp-dx	jdx dx\n\
chemical/x-kinemage	kin\n\
chemical/x-macmolecule	mcm\n\
chemical/x-macromodel-input	mmd mmod\n\
chemical/x-mdl-molfile	mol\n\
chemical/x-mdl-rdfile	rd\n\
chemical/x-mdl-rxnfile	rxn\n\
chemical/x-mdl-sdfile	sd sdf\n\
chemical/x-mdl-tgf	tgf\n\
\n\
chemical/x-mmcif	mcif\n\
chemical/x-mol2	mol2\n\
chemical/x-molconn-Z	b\n\
chemical/x-mopac-graph	gpt\n\
chemical/x-mopac-input	mop mopcrt mpc zmt\n\
chemical/x-mopac-out	moo\n\
chemical/x-mopac-vib	mvb\n\
chemical/x-ncbi-asn1	asn\n\
chemical/x-ncbi-asn1-ascii	prt ent\n\
chemical/x-ncbi-asn1-binary	val aso\n\
chemical/x-ncbi-asn1-spec	asn\n\
chemical/x-pdb	pdb ent\n\
chemical/x-rosdal	ros\n\
chemical/x-swissprot	sw\n\
chemical/x-vamas-iso14976	vms\n\
chemical/x-vmd	vmd\n\
chemical/x-xtel	xtel\n\
chemical/x-xyz	xyz\n\
\n\
image/cgm\n\
image/g3fax\n\
image/gif	gif\n\
image/ief	ief\n\
image/jpeg	jpeg jpg jpe\n\
image/naplps\n\
image/pcx	pcx\n\
image/png	png\n\
image/prs.btif\n\
image/prs.pti\n\
image/svg+xml	svg svgz\n\
image/tiff	tiff tif\n\
image/vnd.cns.inf2\n\
image/vnd.djvu	djvu djv\n\
image/vnd.dwg\n\
image/vnd.dxf\n\
image/vnd.fastbidsheet\n\
image/vnd.fpx\n\
image/vnd.fst\n\
image/vnd.fujixerox.edmics-mmr\n\
image/vnd.fujixerox.edmics-rlc\n\
image/vnd.mix\n\
image/vnd.net-fpx\n\
image/vnd.svf\n\
image/vnd.wap.wbmp	wbmp\n\
image/vnd.xiff\n\
image/x-canon-cr2	cr2\n\
image/x-canon-crw	crw\n\
image/x-cmu-raster	ras\n\
image/x-coreldraw	cdr\n\
image/x-coreldrawpattern	pat\n\
image/x-coreldrawtemplate	cdt\n\
image/x-corelphotopaint	cpt\n\
image/x-epson-erf	erf\n\
image/x-icon	ico\n\
image/x-jg	art\n\
image/x-jng	jng\n\
image/x-ms-bmp	bmp\n\
image/x-nikon-nef	nef\n\
image/x-olympus-orf	orf\n\
image/x-photoshop	psd\n\
image/x-portable-anymap	pnm\n\
image/x-portable-bitmap	pbm\n\
image/x-portable-graymap	pgm\n\
image/x-portable-pixmap	ppm\n\
image/x-rgb	rgb\n\
image/x-xbitmap	xbm\n\
image/x-xpixmap	xpm\n\
image/x-xwindowdump	xwd\n\
\n\
inode/chardevice\n\
inode/blockdevice\n\
inode/directory-locked\n\
inode/directory\n\
inode/fifo\n\
inode/socket\n\
\n\
message/delivery-status\n\
message/disposition-notification\n\
message/external-body\n\
message/http\n\
message/s-http\n\
message/news\n\
message/partial\n\
message/rfc822	eml\n\
\n\
model/iges	igs iges\n\
model/mesh	msh mesh silo\n\
model/vnd.dwf\n\
model/vnd.flatland.3dml\n\
model/vnd.gdl\n\
model/vnd.gs-gdl\n\
model/vnd.gtw\n\
model/vnd.mts\n\
model/vnd.vtu\n\
model/vrml	wrl vrml\n\
model/x3d+vrml	x3dv\n\
model/x3d+xml	x3d\n\
model/x3d+binary	x3db\n\
\n\
multipart/alternative\n\
multipart/appledouble\n\
multipart/byteranges\n\
multipart/digest\n\
multipart/encrypted\n\
multipart/form-data\n\
multipart/header-set\n\
multipart/mixed\n\
multipart/parallel\n\
multipart/related\n\
multipart/report\n\
multipart/signed\n\
multipart/voice-message\n\
\n\
text/cache-manifest	manifest\n\
text/calendar	ics icz\n\
text/css	css\n\
text/csv	csv\n\
text/directory\n\
text/english\n\
text/enriched\n\
text/h323	323\n\
text/html	html htm shtml\n\
text/iuls	uls\n\
text/mathml	mml\n\
text/parityfec\n\
text/plain	asc txt text pot brf\n\
text/prs.lines.tag\n\
text/rfc822-headers\n\
text/richtext	rtx\n\
text/rtf\n\
text/scriptlet	sct wsc\n\
text/t140\n\
text/texmacs	tm\n\
text/tab-separated-values	tsv\n\
text/uri-list\n\
text/vnd.abc\n\
text/vnd.curl\n\
text/vnd.DMClientScript\n\
text/vnd.flatland.3dml\n\
text/vnd.fly\n\
text/vnd.fmi.flexstor\n\
text/vnd.in3d.3dml\n\
text/vnd.in3d.spot\n\
text/vnd.IPTC.NewsML\n\
text/vnd.IPTC.NITF\n\
text/vnd.latex-z\n\
text/vnd.motorola.reflex\n\
text/vnd.ms-mediapackage\n\
text/vnd.sun.j2me.app-descriptor	jad\n\
text/vnd.wap.si\n\
text/vnd.wap.sl\n\
text/vnd.wap.wml	wml\n\
text/vnd.wap.wmlscript	wmls\n\
text/x-bibtex	bib\n\
text/x-boo	boo\n\
text/x-c++hdr	h++ hpp hxx hh\n\
text/x-c++src	c++ cpp cxx cc\n\
text/x-chdr	h\n\
text/x-component	htc\n\
text/x-crontab\n\
text/x-csh	csh\n\
text/x-csrc	c\n\
text/x-dsrc	d\n\
text/x-diff	diff patch\n\
text/x-haskell	hs\n\
text/x-java	java\n\
text/x-literate-haskell	lhs\n\
text/x-makefile\n\
text/x-moc	moc\n\
text/x-pascal	p pas\n\
text/x-pcs-gcd	gcd\n\
text/x-perl	pl pm\n\
text/x-python	py\n\
text/x-scala	scala\n\
text/x-server-parsed-html\n\
text/x-setext	etx\n\
text/x-sfv	sfv\n\
text/x-sh	sh\n\
text/x-tcl	tcl tk\n\
text/x-tex	tex ltx sty cls\n\
text/x-vcalendar	vcs\n\
text/x-vcard	vcf\n\
\n\
video/3gpp	3gp\n\
video/annodex	axv\n\
video/dl	dl\n\
video/dv	dif dv\n\
video/fli	fli\n\
video/gl	gl\n\
video/mpeg	mpeg mpg mpe\n\
video/MP2T	ts\n\
video/mp4	mp4\n\
video/quicktime	qt mov\n\
video/mp4v-es\n\
video/ogg	ogv\n\
video/parityfec\n\
video/pointer\n\
video/webm	webm\n\
video/vnd.fvt\n\
video/vnd.motorola.video\n\
video/vnd.motorola.videop\n\
video/vnd.mpegurl	mxu\n\
video/vnd.mts\n\
video/vnd.nokia.interleaved-multimedia\n\
video/vnd.vivo\n\
video/x-flv	flv\n\
video/x-la-asf	lsf lsx\n\
video/x-mng	mng\n\
video/x-ms-asf	asf asx\n\
video/x-ms-wm	wm\n\
video/x-ms-wmv	wmv\n\
video/x-ms-wmx	wmx\n\
video/x-ms-wvx	wvx\n\
video/x-msvideo	avi\n\
video/x-sgi-movie	movie\n\
video/x-matroska	mpv mkv\n\
\n\
x-conference/x-cooltalk	ice\n\
\n\
x-epoc/x-sisx-app	sisx\n\
x-world/x-vrml	vrm vrml wrl";

#endif //MIME_FILE__H_
