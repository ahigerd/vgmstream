#include "meta.h"
#include "../util.h"

/* NGCA (from GoldenEye 007) */
VGMSTREAM * init_vgmstream_ngca(STREAMFILE *streamFile) {
    VGMSTREAM * vgmstream = NULL;
    char filename[PATH_LIMIT];
    off_t start_offset;

    int loop_flag;
   int channel_count;

    /* check extension, case insensitive */
    streamFile->get_name(streamFile,filename,sizeof(filename));
    if (strcasecmp("ngca",filename_extension(filename))) goto fail;

    /* check header */
    if (read_32bitBE(0x00,streamFile) != 0x4E474341) /* "NGCA" */
        goto fail;

    loop_flag = 0;
    channel_count = 1;

   /* build the VGMSTREAM */
    vgmstream = allocate_vgmstream(channel_count,loop_flag);
    if (!vgmstream) goto fail;

   /* fill in the vital statistics */
    start_offset = 0x40;
    vgmstream->channels = channel_count;
    vgmstream->sample_rate = 32000;
    vgmstream->coding_type = coding_NGC_DSP;
    vgmstream->num_samples = (((read_32bitBE(0x4,streamFile))/2) - 1) / 8 * 14;

    vgmstream->layout_type = layout_none;
    vgmstream->meta_type = meta_NGCA;
    vgmstream->allow_dual_stereo = 1;

	 if (vgmstream->coding_type == coding_NGC_DSP) {
         int i;
         for (i=0;i<16;i++) {
            vgmstream->ch[0].adpcm_coef[i] = read_16bitBE(0xC+i*2,streamFile);
        }
    }

    /* open the file for reading */
    {
        int i;
        STREAMFILE * file;
        file = streamFile->open(streamFile,filename,STREAMFILE_DEFAULT_BUFFER_SIZE);
        if (!file) goto fail;
        for (i=0;i<channel_count;i++) {
            vgmstream->ch[i].streamfile = file;

            vgmstream->ch[i].channel_start_offset=
                vgmstream->ch[i].offset=start_offset+
                vgmstream->interleave_block_size*i;

        }
    }

    return vgmstream;

    /* clean up anything we may have opened */
fail:
    if (vgmstream) close_vgmstream(vgmstream);
    return NULL;
}
