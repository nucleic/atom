.. _advanced-manual-notifications:

Manual notifications
====================

.. include:: ../substitutions.sub

Atom object usually fire notifications at the proper times. However, in some
cases (Property member, manual handling of container change), it may be
desirable to manually fire a notification.

This is possible but require some care.

First, when manually notifying, you are responsible for building the change
dictionary that will be passed to the handlers. You may refer to
:ref:`basis-observation` for a description of the content of this dictionary
for normal notifications.

Second, because atom handle separately the static observers and the dynamic
observers, you will to be sure to call both kinds. To notify the static
observers, you should call the |Member.notify| method, while to notify
dynamic obsevers you need to call the |Atom.notify| method on the instance.
