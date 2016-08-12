#ifndef _PAXOS_TEST_H_
#define _PAXOS_TEST_H_

int32_t get_acceptor(struct px_acceptor **acpt, struct px_acpt_info *acptInfo);
void    put_acceptor(struct px_acceptor *acpt);
int32_t get_proposer(struct px_proposer **ppr, struct px_ppr_info *pprInfo);
void    put_proposer(struct px_proposer *ppr);
int32_t get_learner(struct px_learner **lnr, struct px_lnr_info *lnrInfo);
void    put_learner(struct px_learner *lnr);

#endif

