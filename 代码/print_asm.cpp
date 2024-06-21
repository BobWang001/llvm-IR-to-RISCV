#include "riscv.h"

void print_asm(functions* now_func)
{
	asm_instruction* asm_ins_head = now_func->asm_ins_head;
	while (asm_ins_head->next != NULL)
	{
		asm_ins_head = asm_ins_head->next;
		switch (asm_ins_head->op)
		{
            case ins_asm_mv:
            {
                printf("    mv %s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str());
                break;
            }
            case ins_asm_la:
            {
                printf("    la %s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    ("G" + std::to_string(asm_ins_head->label)).c_str());
                break;
            }
            case ins_asm_li:
            {
                printf("    li %s,%d\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    asm_ins_head->imm);
                break;
            }
            case ins_asm_lw:
            {
                printf("    lw %s,%d(%s)\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    asm_ins_head->imm, map_physical_reg_name[asm_ins_head->Rs1].c_str());
                break;
            }
            case ins_asm_flw:
            {
                printf("    flw %s,%d(%s)\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    asm_ins_head->imm, map_physical_reg_name[asm_ins_head->Rs1].c_str());
                break;
            }
            case ins_asm_ld:
            {
                printf("    ld %s,%d(%s)\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    asm_ins_head->imm, map_physical_reg_name[asm_ins_head->Rs1].c_str());
                break;
            }
            case ins_asm_fld:
            {
                printf("    fld %s,%d(%s)\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    asm_ins_head->imm, map_physical_reg_name[asm_ins_head->Rs1].c_str());
                break;
            }
            case ins_asm_sw:
            {
                printf("    sw %s,%d(%s)\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    asm_ins_head->imm, map_physical_reg_name[asm_ins_head->Rs1].c_str());
                break;
            }
            case ins_asm_fsw:
            {
                printf("    fsw %s,%d(%s)\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    asm_ins_head->imm, map_physical_reg_name[asm_ins_head->Rs1].c_str());
                break;
            }
            case ins_asm_sd:
            {
                printf("    sd %s,%d(%s)\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    asm_ins_head->imm, map_physical_reg_name[asm_ins_head->Rs1].c_str());
                break;
            }
            case ins_asm_fsd:
            {
                printf("    fsd %s,%d(%s)\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    asm_ins_head->imm, map_physical_reg_name[asm_ins_head->Rs1].c_str());
                break;
            }
            case ins_asm_fmv_x_w:
            {
                printf("    fmv.x.w %s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str());
                break;
            }
            case ins_asm_fmv_w_x:
            {
                printf("    fmv.w.x %s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str());
                break;
            }
            case ins_asm_add:
            {
                printf("    add %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_addi:
            {
                printf("    addi %s,%s,%d\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(), asm_ins_head->imm);
                break;
            }
            case ins_asm_sub:
            {
                printf("    sub %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_subi:
            {
                printf("    subi %s,%s,%d\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(), asm_ins_head->imm);
                break;
            }
            case ins_asm_mul:
            {
                printf("    mul %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_div:
            {
                printf("    div %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_rem:
            {
                printf("    rem %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_and:
            {
                printf("    and %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_andi:
            {
                printf("    andi %s,%s,%d\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(), asm_ins_head->imm);
                break;
            }
            case ins_asm_or:
            {
                printf("    or %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_ori:
            {
                printf("    ori %s,%s,%d\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(), asm_ins_head->imm);
                break;
            }
            case ins_asm_xor:
            {
                printf("    xor %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_xori:
            {
                printf("    xori %s,%s,%d\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(), asm_ins_head->imm);
                break;
            }
            case ins_asm_sll:
            {
                printf("    sll %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_srl:
            {
                printf("    srl %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_sra:
            {
                printf("    sra %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_slli:
            {
                printf("    slli %s,%s,%d\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(), asm_ins_head->imm);
                break;
            }
            case ins_asm_srli:
            {
                printf("    srli %s,%s,%d\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(), asm_ins_head->imm);
                break;
            }
            case ins_asm_srai:
            {
                printf("    srai %s,%s,%d\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(), asm_ins_head->imm);
                break;
            }
            case ins_asm_addw:
            {
                printf("    addw %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_addiw:
            {
                printf("    addiw %s,%s,%d\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(), asm_ins_head->imm);
                break;
            }
            case ins_asm_subw:
            {
                printf("    subw %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_subiw:
            {
                printf("    subiw %s,%s,%d\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(), asm_ins_head->imm);
                break;
            }
            case ins_asm_mulw:
            {
                printf("    mulw %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_divw:
            {
                printf("    divw %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_remw:
            {
                printf("    remw %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_andw:
            {
                printf("    andw %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_andiw:
            {
                printf("    andiw %s,%s,%d\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(), asm_ins_head->imm);
                break;
            }
            case ins_asm_orw:
            {
                printf("    orw %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_oriw:
            {
                printf("    oriw %s,%s,%d\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(), asm_ins_head->imm);
                break;
            }
            case ins_asm_xorw:
            {
                printf("    xorw %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_xoriw:
            {
                printf("    xoriw %s,%s,%d\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(), asm_ins_head->imm);
                break;
            }
            case ins_asm_sllw:
            {
                printf("    sllw %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_srlw:
            {
                printf("    srlw %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_sraw:
            {
                printf("    sraw %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_slliw:
            {
                printf("    slliw %s,%s,%d\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(), asm_ins_head->imm);
                break;
            }
            case ins_asm_srliw:
            {
                printf("    srliw %s,%s,%d\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(), asm_ins_head->imm);
                break;
            }
            case ins_asm_sraiw:
            {
                printf("    sraiw %s,%s,%d\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(), asm_ins_head->imm);
                break;
            }
            case ins_asm_fadd_s:
            {
                printf("    fadd.s %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_fsub_s:
            {
                printf("    fsub.s %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_fmul_s:
            {
                printf("    fmul.s %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_fdiv_s:
            {
                printf("    fdiv.s %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_frem_s:
            {
                printf("    frem.s %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_fneg_s:
            {
                printf("    fneg.s %s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str());
                break;
            }
            case ins_asm_fcvt_s_w:
            {
                printf("    fcvt.s.w %s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str());
                break;
            }
            case ins_asm_fcvt_w_s:
            {
                printf("    fcvt.w.s %s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str());
                break;
            }
            case ins_asm_j:
            {
                printf("    j %s\n", ("L" + std::to_string(asm_ins_head->label)).c_str());
                break;
            }
            case ins_asm_jal:
            {
                printf("    jal %s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    ("L" + std::to_string(asm_ins_head->label)).c_str());
                break;
            }
            case ins_asm_jr:
            {
                printf("    jr %s\n", map_physical_reg_name[asm_ins_head->Rs1].c_str());
                break;
            }
            case ins_asm_beq:
            {
                printf("    beq %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str(),
                    ("L" + std::to_string(asm_ins_head->label)).c_str());
                break;
            }
            case ins_asm_bne:
            {
                printf("    bne %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str(),
                    ("L" + std::to_string(asm_ins_head->label)).c_str());
                break;
            }
            case ins_asm_blt:
            {
                printf("    blt %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str(),
                    ("L" + std::to_string(asm_ins_head->label)).c_str());
                break;
            }
            case ins_asm_bltu:
            {
                printf("    bltu %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str(),
                    ("L" + std::to_string(asm_ins_head->label)).c_str());
                break;
            }
            case ins_asm_ble:
            {
                printf("    ble %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str(),
                    ("L" + std::to_string(asm_ins_head->label)).c_str());
                break;
            }
            case ins_asm_bleu:
            {
                printf("    bleu %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str(),
                    ("L" + std::to_string(asm_ins_head->label)).c_str());
                break;
            }
            case ins_asm_bgt:
            {
                printf("    bgt %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str(),
                    ("L" + std::to_string(asm_ins_head->label)).c_str());
                break;
            }
            case ins_asm_bgtu:
            {
                printf("    bgtu %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str(),
                    ("L" + std::to_string(asm_ins_head->label)).c_str());
                break;
            }
            case ins_asm_bge:
            {
                printf("    bge %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str(),
                    ("L" + std::to_string(asm_ins_head->label)).c_str());
                break;
            }
            case ins_asm_bgeu:
            {
                printf("    bgeu %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str(),
                    ("L" + std::to_string(asm_ins_head->label)).c_str());
                break;
            }
            case ins_asm_feq_s:
            {
                printf("    feq.s %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_flt_s:
            {
                printf("    flt.s %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_fle_s:
            {
                printf("    fle.s %s,%s,%s\n", map_physical_reg_name[asm_ins_head->Rd].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_call:
            {
                printf("    call %s\n", ("F" + std::to_string(asm_ins_head->label)).c_str());
                break;
            }
            case ins_asm_fcmpe_s:
            {
                printf("    fcmpe.s %s,%s\n", map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    map_physical_reg_name[asm_ins_head->Rs2].c_str());
                break;
            }
            case ins_asm_bgtz:
            {
                printf("    bgtz %s,%s\n", map_physical_reg_name[asm_ins_head->Rs1].c_str(),
                    ("L" + std::to_string(asm_ins_head->label)).c_str());
                break;
            }
            case ins_asm_label:
            {
                printf("%s:\n", ("L" + std::to_string(asm_ins_head->label)).c_str());
                break;
            }
		}
	}
}