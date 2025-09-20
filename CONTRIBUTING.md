# Contributing to userspace-resource-moderator

Hi there!
We’re thrilled that you’d like to contribute to this project.
Your help is essential for keeping this project great and for making it better.

## Branching Strategy

Contributors should develop on branches based off of `main` and pull requests should be made against `main`.

## Submitting a pull request

1. Please read our [code of conduct](CODE-OF-CONDUCT.md) and [license](LICENSE.txt).
2. [Fork](https://github.com/qualcomm/userspace-resource-moderator/fork) and clone the repository.

    ```bash
    git clone https://github.com/<username>/userspace-resource-moderator.git
    ```

3. Create a new branch based on `main`:

    ```bash
    git checkout -b <my-branch-name> main
    ```

4. Create an upstream `remote` to make it easier to keep your branches up-to-date:

    ```bash
    git remote add upstream https://github.com/qualcomm/userspace-resource-moderator.git
    ```

5. Make your changes, add tests if needed, and make sure the existing tests still pass.
6. Commit your changes using the [DCO](https://developercertificate.org/). You can attest to the DCO by commiting with the **-s** or **--signoff** options or manually adding the "Signed-off-by":

    ```bash
    git commit -s -m "Really useful commit message"`
    ```

7. After committing your changes on the topic branch, sync it with the upstream branch:

    ```bash
    git pull origin main
    ```

8. Push to your fork.

    ```bash
    git push -u origin <my-branch-name>
    ```

    The `-u` is shorthand for `--set-upstream`. This will set up the tracking reference so subsequent runs of `git push` or `git pull` can omit the remote and branch.

9. [Submit a pull request](https://github.com/qualcomm/userspace-resource-moderator/pulls) from your branch to `main`.
10. Await PR review.

Here are a few things you can do that will increase the likelihood of your pull request to be accepted:

- Follow the existing code style and practices as much as possible.
- Write tests.
- Keep your change as focused as possible.
  If you want to make multiple independent changes, please consider submitting them as separate pull requests.
- Write a [good commit message](https://tbaggery.com/2008/04/19/a-note-about-git-commit-messages.html).
- It's a good idea to arrange a discussion with other developers to ensure there is consensus on large features, architecture changes, and other core code changes. PR reviews will go much faster when there are no surprises.
