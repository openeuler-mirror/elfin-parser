// Copyright (c) 2013 Austin T. Clements. All rights reserved.
// Use of this source code is governed by an MIT license
// that can be found in the LICENSE file.

#include "internal.hh"
#include "../elf/sig_handler.hh"

#include <cassert>
#include <string.h>

using namespace std;

DWARFPP_BEGIN_NAMESPACE

// The expected number of arguments for standard opcodes.  This is
// used to check the opcode_lengths header field for compatibility.
static const int opcode_lengths[] = {
        0,
        // DW_LNS::copy
        0, 1, 1, 1, 1,
        // DW_LNS::negate_stmt
        0, 0, 0, 1, 0,
        // DW_LNS::set_epilogue_begin
        0, 1
};

line_table::format_entry::format_entry(cursor &cur)
        : m_type(static_cast<DW_LNCT>(cur.uleb128())),
        m_form(static_cast<DW_FORM>(cur.uleb128()))
{
}

void line_table::directory_format::init(cursor &cur)
{
        auto count = cur.fixed<ubyte>();
        if (count != 1) {
                // to caught usecase for directory with optional parameters
                throw format_error("unexpected directory format entry count " + ::std::to_string(count));
        }
        auto &entry = emplace_back(cur);
        if (entry.type() != DW_LNCT::path) {
                // to caught usecase for directory with optional parameters
                throw format_error("unexpected directory format entry type " + to_string(entry.type()));
        }
        switch (entry.form()) {
        case DW_FORM::string:
        case DW_FORM::line_strp:
        case DW_FORM::strp:
        case DW_FORM::strp_sup:
        case DW_FORM::strx:
        case DW_FORM::strx1:
        case DW_FORM::strx2:
        case DW_FORM::strx4:
                break;
        default:
                throw format_error("unexpected directory format entry form " + to_string(entry.form()));
        }
}

void line_table::file_format::init(cursor &cur)
{
        auto count = cur.fixed<ubyte>();
        if (!count) {
                throw format_error("unexpected file format entry count 0");
        }
        for (auto i = 0; i < count; ++i) {
                auto &entry = emplace_back(cur);
                bool bFail = false;
                switch (entry.type()) {
                case DW_LNCT::path:
                        switch (entry.form()) {
                        case DW_FORM::string:
                        case DW_FORM::line_strp:
                        case DW_FORM::strp:
                        case DW_FORM::strp_sup:
                        case DW_FORM::strx:
                        case DW_FORM::strx1:
                        case DW_FORM::strx2:
                        case DW_FORM::strx4:
                                break;
                        default:
                                bFail = true;
                                break;
                        }
                        break;
                case DW_LNCT::directory_index:
                        switch (entry.form()) {
                        case DW_FORM::data1:
                        case DW_FORM::data2:
                        case DW_FORM::udata:
                                break;
                        default:
                                bFail = true;
                                break;
                        }
                        break;
                case DW_LNCT::timestamp:
                        switch (entry.form()) {
                        case DW_FORM::data4:
                        case DW_FORM::data8:
                        case DW_FORM::udata:
                        case DW_FORM::block:
                                break;
                        default:
                                bFail = true;
                                break;
                        }
                        break;
                case DW_LNCT::size:
                        switch (entry.form()) {
                        case DW_FORM::data1:
                        case DW_FORM::data2:
                        case DW_FORM::data4:
                        case DW_FORM::data8:
                        case DW_FORM::udata:
                                break;
                        default:
                                bFail = true;
                                break;
                        }
                        break;
                case DW_LNCT::md5:
                        bFail = entry.form() != DW_FORM::data16;
                        break;
                }
                if (bFail) {
                        throw format_error("unexpected file format entry type " + to_string(entry.type()) + " form " + to_string(entry.form()));
                }
        }
}

line_table::directory::directory(const ::std::string &path) : m_path(path)
{
        validate_path();
}

line_table::directory::directory(dwarf_cursor &cur, const format &format)
{
        // in current implementation the directory_format::size shall be 1
        // in current implementation the directory_format format_entry::type shall be DW_LNCT::path
        m_path = cur.cstr(format.front().form());
}

line_table::directory::directory(dwarf_cursor &cur, const ::std::string &comp_dir)
{
        cur.string(m_path);
        validate_path(comp_dir);
}

line_table::directory::directory(const ::std::string &path, const ::std::string &comp_dir) : m_path(path)
{
        validate_path(comp_dir);
}

const std::string &line_table::directory::path() const
{
        return m_path;
}

void line_table::directory::validate_path()
{
        if (m_path.empty()) {
                throw format_error("empty directory");
        }
        if (m_path.back() != '/') {
                m_path += '/';
        }
}

void line_table::directory::validate_path(const ::std::string &comp_dir)
{
        validate_path();
        if (m_path.front() != '/') {
                m_path = comp_dir + m_path;
        }
}

line_table::file::file(dwarf_cursor &cur, const format &format)
{
        for (auto &format_entry : format) {
                switch (format_entry.type()) {
                case DW_LNCT::path:
                        m_path = cur.cstr(format_entry.form());
                        break;
                case DW_LNCT::directory_index:
                        m_directory_index = cur.fixed<decltype(m_directory_index)>(format_entry.form());
                        break;
                case DW_LNCT::timestamp:
                        m_time = cur.fixed<decltype(m_time)>(format_entry.form());
                        break;
                case DW_LNCT::size:
                        m_length = cur.fixed<decltype(m_length)>(format_entry.form());
                        break;
                case DW_LNCT::md5:
                        m_md5 = cur.pos;
                        cur.skip_form(format_entry.form());
                        break;
                }
        }
}

line_table::file::file(dwarf_cursor &cur, const format &format, directory_list &dirs)
{
        for (auto &format_entry : format) {
                switch (format_entry.type()) {
                case DW_LNCT::path:
                        m_path = cur.cstr(format_entry.form());
                        break;
                case DW_LNCT::directory_index:
                        m_directory_index = cur.fixed<decltype(m_directory_index)>(format_entry.form());
                        break;
                case DW_LNCT::timestamp:
                        m_time = cur.fixed<decltype(m_time)>(format_entry.form());
                        break;
                case DW_LNCT::size:
                        m_length = cur.fixed<decltype(m_length)>(format_entry.form());
                        break;
                case DW_LNCT::md5:
                        m_md5 = cur.pos;
                        cur.skip_form(format_entry.form());
                        break;
                }
        }
        if (m_directory_index >= dirs.size())
        {
                validate_path();
        } else {
                std::string dir = dirs[m_directory_index].path();
                validate_path(dir);
        }
}

line_table::file::file(const ::std::string &file, const ::std::string &comp_dir)
{
        m_path = file;
        validate_path(comp_dir);
}

line_table::file::file(const ::std::string &file)
{
        m_path = file;
        validate_path();
}

line_table::file::file(dwarf_cursor &cur, const ::std::string &comp_dir, directory_list &dirs)
{
        cur.string(m_path);
        m_directory_index = cur.uleb128();
        m_time = cur.uleb128();
        m_length = cur.uleb128();
        size_t dir_index = size_t(m_directory_index);
        if (dir_index >= dirs.size()) {
            validate_path(comp_dir);
        } else {
            std::string dir = dirs[dir_index].path();
            validate_path(dir);
        }
}

void line_table::file::validate_path()
{
        if (m_path.empty()) {
                throw format_error("empty file");
        }
}

void line_table::file::validate_path(const ::std::string &comp_dir)
{
        validate_path();
        if (m_path.front() != '/') {
                m_path = comp_dir + m_path;
        } else {
                m_path = m_path;
        }
}

template<typename ItemT>
void line_table::path_list<ItemT>::init(dwarf_cursor &cur, const format &format)
{
        auto count = cur.uleb128();
        if (!count) {
                throw format_error("unexpected path count 0");
        }
        for (auto i = 0; i < count; ++i) {
                emplace_back(cur, format);
        }
}

template<typename ItemT>
void line_table::path_list<ItemT>::init(dwarf_cursor &cur, const format &format, directory_list& dirs)
{
        auto count = cur.uleb128();
        if (!count) {
                throw format_error("unexpected path count 0");
        }
        for (auto i = 0; i < count; ++i) {
                emplace_back(cur, format, dirs);
        }
}

template<typename ItemT>
void line_table::path_list<ItemT>::init(dwarf_cursor &cur, const ::std::string &comp_dir)
{
        while (*cur.pos) {
                emplace_back(cur, comp_dir);
        }
        ++cur.pos;
}

template<typename ItemT>
void line_table::path_list<ItemT>::init(dwarf_cursor &cur, const ::std::string &comp_dir, directory_list& dirs)
{
        while (*cur.pos) {
                emplace_back(cur, comp_dir, dirs);
        }
        ++cur.pos;
}

void line_table::directory_list::init(dwarf_cursor &cur, const ::std::string &comp_dir)
{
        emplace_back(comp_dir);
        std::string incdir;
        while(true)
        {
            cur.string(incdir);
            if(incdir.empty()){
                break;
            }
            if(incdir.back() != '/'){
                incdir += '/';
            }
            if(incdir[0] == '/'){
                emplace_back(move(incdir));
            }else{
                emplace_back(comp_dir + incdir);
            }
        }
}

void line_table::file_list::init(dwarf_cursor &cur, const ::std::string &comp_dir, const ::std::string &cu_name, directory_list &dirs)
{
        emplace_back(cu_name, comp_dir);
        path_list<file>::init(cur, comp_dir, dirs);
}

struct line_table::impl
{
        shared_ptr<section> sec;

        // Header information
        section_offset program_offset;
        ubyte minimum_instruction_length;
        ubyte maximum_operations_per_instruction;
        bool default_is_stmt;
        sbyte line_base;
        ubyte line_range;
        ubyte opcode_base;
        vector<ubyte> standard_opcode_lengths;
        directory_format m_directory_format {};
        file_format m_file_format {};
        directory_list m_directories {};
        file_list m_files {};

        // The offset in sec following the last read file name entry.
        // File name entries can appear both in the line table header
        // and in the line number program itself.  Since we can
        // iterate over the line number program repeatedly, this keeps
        // track of how far we've gotten so we don't add the same
        // entry twice.
        section_offset last_file_name_end;
        // If an iterator has traversed the entire program, then we
        // know we've gathered all file names.
        bool file_names_complete;

        impl() : last_file_name_end(0), file_names_complete(false) {};
};

line_table::line_table(const compilation_unit &cu, const shared_ptr<section> &sec, section_offset offset)
        : m(make_shared<impl>())
{
        // XXX DWARF2 and 3 give a weird specification for DW_AT_comp_dir
        signal(SIGABRT, sigabrt_handler);
        signal(SIGSEGV, sigsegv_handler);
        string abs_path;

        // Read the line table header (DWARF2 section 6.2.4, DWARF3
        // section 6.2.4, DWARF4 section 6.2.3, DWARF5 section 6.2.4)
        m->sec = cursor(sec, offset).subsection();
        dwarf_cursor cur(cu.get_dwarf(), m->sec);
        cur.skip_initial_length();

        // Basic header information
        uhalf version = cur.fixed<uhalf>();
        if (version < 2 || version > 5)
                throw format_error("unknown line number table version " +
                                   std::to_string(version));
        if (version == 5) {
                m->sec->addr_size = cur.fixed<ubyte>();
                m->sec->segment_selector_size = cur.fixed<ubyte>();
        } else {
                m->sec->addr_size = cu.address_size();
                m->sec->segment_selector_size = 0;
        }
        section_length header_length = cur.offset();
        m->program_offset = cur.get_section_offset() + header_length;
        m->minimum_instruction_length = cur.fixed<ubyte>();
        if (version >= 4) {
                m->maximum_operations_per_instruction = cur.fixed<ubyte>();
        } else {
                m->maximum_operations_per_instruction = 1;
        }
        if (m->maximum_operations_per_instruction == 0)
                throw format_error("maximum_operations_per_instruction cannot"
                                   " be 0 in line number table");
        m->default_is_stmt = cur.fixed<ubyte>();
        m->line_base = cur.fixed<sbyte>();
        m->line_range = cur.fixed<ubyte>();
        if (m->line_range == 0)
                throw format_error("line_range cannot be 0 in line number table");
        m->opcode_base = cur.fixed<ubyte>();
        
        static_assert(sizeof(opcode_lengths) / sizeof(opcode_lengths[0]) == 13,
                      "opcode_lengths table has wrong length");

        // Opcode length table
        m->standard_opcode_lengths.resize(m->opcode_base);
        m->standard_opcode_lengths[0] = 0;
        for (unsigned i = 1; i < m->opcode_base; i++) {
                ubyte length = cur.fixed<ubyte>();
                if (length != opcode_lengths[i])
                        // The spec never says what to do if the
                        // opcode length of a standard opcode doesn't
                        // match the header.  Do the safe thing.
                        throw format_error(
                                "expected " +
                                std::to_string(opcode_lengths[i]) +
                                " arguments for line number opcode " +
                                std::to_string(i) + ", got " +
                                std::to_string(length));
                m->standard_opcode_lengths[i] = length;
        }

        if (version == 5) {
                m->m_directory_format.init(cur);
                m->m_directories.init(cur, m->m_directory_format);
                m->m_file_format.init(cur);
                m->m_files.init(cur, m->m_file_format, m->m_directories);
        } else {
                auto comp_dir =cu.comp_dir();
                m->m_directories.init(cur, comp_dir);
                m->m_files.init(cur, comp_dir, cu.name(), m->m_directories);
        }
}

line_table::iterator
line_table::begin() const
{
        if (!valid())
                return iterator(nullptr, 0);
        return iterator(this, m->program_offset);
}

line_table::iterator
line_table::end() const
{
        if (!valid())
                return iterator(nullptr, 0);
        return iterator(this, m->sec->size());
}

line_table::iterator
line_table::find_address(taddr addr) const
{
        iterator prev = begin(), e = end();
        if (prev == e)
                return prev;

        iterator it = prev;
        for (++it; it != e; prev = it++) {
                if (prev->address <= addr && it->address > addr &&
                    !prev->end_sequence)
                        return prev;
        }
        prev = e;
        return prev;
}

const line_table::file *
line_table::get_file(unsigned index) const
{
        if (index >= m->m_files.size()) {
                // It could be declared in the line table program.
                // This is unlikely, so we don't have to be
                // super-efficient about this.  Just force our way
                // through the whole line table program.
                if (!m->file_names_complete) {
                        for (auto &ent : *this)
                                (void)ent;
                }
                if (index >= m->m_files.size())
                        throw out_of_range
                                ("file name index " + std::to_string(index) +
                                 " exceeds file table size of " +
                                 std::to_string(m->m_files.size()));
        }
        return &m->m_files[index];
}

void
line_table::entry::reset(bool is_stmt)
{
        address = op_index = 0;
        file = nullptr;
        file_index = line = 1;
        column = 0;
        this->is_stmt = is_stmt;
        basic_block = end_sequence = prologue_end = epilogue_begin = false;
        isa = discriminator = 0;
}

string
line_table::entry::get_description() const
{
        string res = file->path();
        if (line) {
                res.append(":").append(std::to_string(line));
                if (column)
                        res.append(":").append(std::to_string(column));
        }
        return res;
}

line_table::iterator::iterator(const line_table *table, section_offset pos)
        : table(table), pos(pos)
{
        if (table) {
                regs.reset(table->m->default_is_stmt);
                ++(*this);
        }
}

line_table::iterator &
line_table::iterator::operator++()
{
        cursor cur(table->m->sec, pos);

        // Execute opcodes until we reach the end of the stream or an
        // opcode emits a line table row
        bool stepped = false, output = false;
        while (!cur.end() && !output) {
                output = step(&cur);
                stepped = true;
        }
        if (stepped && !output)
                throw format_error("unexpected end of line table");
        if (stepped && cur.end()) {
                // Record that all file names must be known now
                table->m->file_names_complete = true;
        }
        if (output) {
                // Resolve file name of entry
                if (entry.file_index < table->m->m_files.size())
                        entry.file = &table->m->m_files[entry.file_index];
                else
                        throw format_error("bad file index " +
                                           std::to_string(entry.file_index) +
                                           " in line table");
        }

        pos = cur.get_section_offset();
        return *this;
}

bool
line_table::iterator::step(cursor *cur)
{
        struct line_table::impl *m = table->m.get();

        // Read the opcode (DWARF4 section 6.2.3)
        ubyte opcode = cur->fixed<ubyte>();
        if (opcode >= m->opcode_base) {
                // Special opcode (DWARF4 section 6.2.5.1)
                ubyte adjusted_opcode = opcode - m->opcode_base;
                unsigned op_advance = adjusted_opcode / m->line_range;
                signed line_inc = m->line_base + (signed)adjusted_opcode % m->line_range;

                regs.line += line_inc;
                regs.address += m->minimum_instruction_length *
                        ((regs.op_index + op_advance)
                         / m->maximum_operations_per_instruction);
                regs.op_index = (regs.op_index + op_advance)
                        % m->maximum_operations_per_instruction;
                entry = regs;

                regs.basic_block = regs.prologue_end =
                        regs.epilogue_begin = false;
                regs.discriminator = 0;

                return true;
        } else if (opcode != 0) {
                // Standard opcode (DWARF4 sections 6.2.3 and 6.2.5.2)
                //
                // According to the standard, any opcode between the
                // highest defined opcode for a given DWARF version
                // and opcode_base should be treated as a
                // vendor-specific opcode. However, the de facto
                // standard seems to be to process these as standard
                // opcodes even if they're from a later version of the
                // standard than the line table header claims.
                uint64_t uarg;
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wswitch-enum"
                switch ((DW_LNS)opcode) {
                case DW_LNS::copy:
                        entry = regs;
                        regs.basic_block = regs.prologue_end =
                                regs.epilogue_begin = false;
                        regs.discriminator = 0;
                        break;
                case DW_LNS::advance_pc:
                        // Opcode advance (as for special opcodes)
                        uarg = cur->uleb128();
                advance_pc:
                        regs.address += m->minimum_instruction_length *
                                ((regs.op_index + uarg)
                                 / m->maximum_operations_per_instruction);
                        regs.op_index = (regs.op_index + uarg)
                                % m->maximum_operations_per_instruction;
                        break;
                case DW_LNS::advance_line:
                        regs.line = (signed)regs.line + cur->sleb128();
                        break;
                case DW_LNS::set_file:
                        regs.file_index = cur->uleb128();
                        break;
                case DW_LNS::set_column:
                        regs.column = cur->uleb128();
                        break;
                case DW_LNS::negate_stmt:
                        regs.is_stmt = !regs.is_stmt;
                        break;
                case DW_LNS::set_basic_block:
                        regs.basic_block = true;
                        break;
                case DW_LNS::const_add_pc:
                        uarg = (255 - m->opcode_base) / m->line_range;
                        goto advance_pc;
                case DW_LNS::fixed_advance_pc:
                        regs.address += cur->fixed<uhalf>();
                        regs.op_index = 0;
                        break;
                case DW_LNS::set_prologue_end:
                        regs.prologue_end = true;
                        break;
                case DW_LNS::set_epilogue_begin:
                        regs.epilogue_begin = true;
                        break;
                case DW_LNS::set_isa:
                        regs.isa = cur->uleb128();
                        break;
                default:
                        // XXX Vendor extensions
                        throw format_error("unknown line number opcode " +
                                           to_string((DW_LNS)opcode));
                }
                return ((DW_LNS)opcode == DW_LNS::copy);
        } else { // opcode == 0
                // Extended opcode (DWARF4 sections 6.2.3 and 6.2.5.3)
                assert(opcode == 0);
                uint64_t length = cur->uleb128();
                section_offset end = cur->get_section_offset() + length;
                opcode = cur->fixed<ubyte>();
                switch ((DW_LNE)opcode) {
                case DW_LNE::end_sequence:
                        regs.end_sequence = true;
                        entry = regs;
                        regs.reset(m->default_is_stmt);
                        break;
                case DW_LNE::set_address:
                        regs.address = cur->address();
                        regs.op_index = 0;
                        break;
                case DW_LNE::define_file:
                        // skip string and 3 uleb128
                        // deprecated in DWARF5 section 6.2.5.3
                        cur->skip_form(DW_FORM::string);
                        cur->skip_form(DW_FORM::udata);
                        cur->skip_form(DW_FORM::udata);
                        cur->skip_form(DW_FORM::udata);
                        break;
                case DW_LNE::set_discriminator:
                        // XXX Only DWARF4
                        regs.discriminator = cur->uleb128();
                        break;
                case DW_LNE::lo_user...DW_LNE::hi_user:
                        // XXX Vendor extensions
                        throw runtime_error("vendor line number opcode " +
                                            to_string((DW_LNE)opcode) +
                                            " not implemented");
                default:
                        // XXX Prior to DWARF4, any opcode number
                        // could be a vendor extension
                        throw format_error("unknown line number opcode " +
                                           to_string((DW_LNE)opcode));
                }
#pragma GCC diagnostic pop
                if (cur->get_section_offset() > end)
                        throw format_error("extended line number opcode exceeded its size");
                cur += end - cur->get_section_offset();
                return ((DW_LNE)opcode == DW_LNE::end_sequence);
        }
}

DWARFPP_END_NAMESPACE
