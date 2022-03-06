	format binary
	use32
	
	include "imports.inc"
	
	
SRC_EXE equ "DFantasy/dfantasy.exe"
PE_BASE equ $400000

SECTION_BASE equ $a00000
SECTION_SIZE equ section_end-section_base

macro incbin_until addr {
	file SRC_EXE:($-PE_BASE),(addr-($-PE_BASE))
}
	
	org PE_BASE
	
	;;;;; add extra section to pe header
	incbin_until $e6
	dw 5 ;5 instead of 4 sections
	
	incbin_until $130
	dd SECTION_BASE + SECTION_SIZE ;extend image size
	
	incbin_until $278
	db "dgcf-en" ;section name
	db (PE_BASE+$280)-$ dup 0
	dd SECTION_SIZE ;virtual size
	dd SECTION_BASE ;virtual address
	dd SECTION_SIZE ;actual size
	dd virtual_section_base-PE_BASE ;raw data pointer
	dd 0 ;no relocations
	dd 0 ;no coff line numbers
	dw 0 ;no relocations
	dw 0 ;no coff line numbers
	dd $e0000060 ;readable, writable, executable, contains code and initialized data
	
	
	incbin_until $300
	db "(C) 2022 Team Kokonoe Media Industries, LLC."
	
	
	
	
	
	file SRC_EXE:($-PE_BASE)
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
virtual_section_base:
	org SECTION_BASE+PE_BASE
section_base:
	
	db "Don't proceed any further! If you want to hack this translation, use the source tools at <https://github.com/karmic64/dgcf-hacktools> instead of your hex editor."
	
		
	
	;this is just some junk code to prove the origin is set correctly
	push eax
	lea eax,[$]
	pop eax
	ret
	
	
	
	
	virtual
		align $1000
section_padding = $ - $$
	end virtual
	db section_padding dup 0
section_end: